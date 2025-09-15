// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Internal logging implementation
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/poll.h>

#include "private.h"
#include "libkalert/libkalert.h"

/* Single thread only */
static uint32_t next_seq(void)
{
	static uint32_t seq = 0;
	return ++seq;
}

/*
 * This function opens a connection to the kernel's kalert
 * module. On error, a negative value is returned. On success,
 * the file descriptor is returned - which can be 0 or higher.
 */
int kalert_open(void)
{
	struct sockaddr_nl local_addr;

	int fd = socket(PF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_KALERT);
	if (fd < 0) {
		if (errno == EINVAL || errno == EPROTONOSUPPORT ||
		    errno == EAFNOSUPPORT)
			kalert_msg(LOG_ERR, "Kalert not support in kernel");
		else
			kalert_msg(LOG_ERR,
				   "Opening kalert netlink socket (%s)",
				   strerror(errno));
	}

	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.nl_family = AF_NETLINK;
	local_addr.nl_pid = getpid(); // Bind to this process's PID

	if (bind(fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
		perror("bind");
		close(fd);
		return -1;
	}

	return fd;
}

void kalert_close(int fd)
{
	if (fd >= 0)
		close(fd);
}

/**
 * kalert_get_reply - receive a netlink reply message from the kernel
 * @fd:    file descriptor of an open netlink socket
 * @rep:   output parameter, pointer to a user-allocated kalert_message buffer
 * @block: whether to wait for data (e.g., REPLY_BLOCK / REPLY_NONBLOCK)
 * @peek:  whether to only peek at the message without removing it (non-zero = MSG_PEEK)
 *
 * Return:
 *   >0  : number of bytes received
 *   =0  : no data available (possible in non-blocking mode)
 *   <0  : error occurred (errno will be set accordingly)
 */
int kalert_get_reply(int fd, struct kalert_message *rep, reply_t block,
		     int peek)
{
	int len;
	struct sockaddr_nl nladdr;
	socklen_t nladdrlen = sizeof(nladdr);

	if (fd < 0)
		return -EBADF;

	if (block == GET_REPLY_NONBLOCKING)
		peek |= MSG_DONTWAIT;

retry:
	len = recvfrom(fd, rep, sizeof(*rep), peek, (struct sockaddr *)&nladdr,
		       &nladdrlen);

	if (len < 0) {
		if (errno == EINTR)
			goto retry;
		if (errno != EAGAIN)
			kalert_msg(LOG_ERR,
				   "Error receiving kalert netlink packet (%s)",
				   strerror(errno));
		return -errno;
	}

	if (nladdrlen != sizeof(nladdr)) {
		kalert_msg(LOG_ERR,
			   "Bad address size reading kalert netlink socket");
		return -EPROTO;
	}

	if (nladdr.nl_pid) {
		kalert_msg(LOG_ERR,
			   "Spoofed packet received on kalert netlink socket");
		return -EINVAL;
	}

	if (!NLMSG_OK(&rep->nlh, (unsigned int)len)) {
		if (len == sizeof(*rep)) {
			kalert_msg(LOG_ERR,
				   "Netlink event from kernel is too big");
			errno = EFBIG;
		} else {
			kalert_msg(LOG_ERR,
				   "Netlink message from kernel was not OK");
			errno = EBADE;
		}
		return -errno;
	}

	return len;
}

/**
 * check_ack - Wait for ACK/error reply matching a given seq
 * @fd:        netlink socket fd
 * @seq:       sequence number of the request we wait for
 *
 * Returns:
 *   0 on success (ACK received)
 *   -ETIMEDOUT if timeout
 *   -errno on other failures
 */
static int check_ack(int fd, int seq)
{
	struct kalert_message rep;
	int timeout_ms = 3000; // wait 3s a time
	int retries = 5; // retry 5 times
	struct pollfd pfd = {
		.fd = fd,
		.events = POLLIN,
	};

	while (retries--) {
		int rc;

		rc = poll(&pfd, 1, timeout_ms < 0 ? -1 : timeout_ms);
		if (rc == 0)
			return -ETIMEDOUT;
		if (rc < 0) {
			if (errno == EINTR)
				continue;
			return -errno;
		}

		/* Peek the next message */
		rc = kalert_get_reply(fd, &rep, GET_REPLY_NONBLOCKING,
				      MSG_PEEK);
		if (rc < 0) {
			if (rc == -EAGAIN)
				continue;
			return rc;
		}

		/* Skip unrelated seq (async notifications, other replies) */
		if (rep.nlh.nlmsg_seq != (unsigned int)seq) {
			/* consume and discard */
			(void)kalert_get_reply(fd, &rep, GET_REPLY_NONBLOCKING,
					       0);
			continue;
		}

		/* It's our ACK/error reply */
		if (rep.nlh.nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *err_msg = NLMSG_DATA(&rep.nlh);
			int error = err_msg->error;
			/* consume */
			(void)kalert_get_reply(fd, &rep, GET_REPLY_NONBLOCKING,
					       0);
			return error ? -error : 0;
		}

		/* consume and ignore other messages */
		(void)kalert_get_reply(fd, &rep, GET_REPLY_NONBLOCKING, 0);
	}

	/* should never reach here */
	return -EIO;
}

static int kalert_send(int fd, struct kalert_message *req, int *seq)
{
	int retval;
	static const struct sockaddr_nl addr = { .nl_family = AF_NETLINK };

	*seq = next_seq();
	req->nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	req->nlh.nlmsg_seq = *seq;

	do {
		retval = sendto(fd, req, req->nlh.nlmsg_len, 0,
				(struct sockaddr *)&addr, sizeof(addr));
	} while (retval < 0 && errno == EINTR);

	return retval;
}

/**
 * kalert_send_request - send a netlink request and wait for acknowledgement
 * @fd:   file descriptor of an open netlink socket
 * @req:  pointer to the kalert_message to send (caller must fill payload)
 *
 * This function wraps kalert_send(), then waits for an ACK reply
 * from the kernel using check_ack(). It ensures the request was
 * delivered and acknowledged.
 *
 * Return:
 *   =0   : request acknowledged successfully
 *   <0   : error occurred (errno will be set accordingly)
 *   >0   : bytes send (number of bytes written)
 */
int kalert_send_request(int fd, struct kalert_message *req)
{
	int rc;
	int seq;

	if (fd < 0 || !req)
		return -EINVAL;

	rc = kalert_send(fd, req, &seq);
	if (rc == (int)req->nlh.nlmsg_len)
		return check_ack(fd, seq);
	return rc;
}
