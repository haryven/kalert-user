// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Implementation of public API
 */

#include <libkalert/libkalert.h>
#include <libmnl/libmnl.h>
#include <stdio.h>

#include "private.h"

#define ATTR_MASK_ALLOWED_SET                                         \
	((1U << KALERT_ENABLE) | (1U << KALERT_PORTID) |              \
	 (1U << KALERT_FILTER_LEVEL) | (1U << KALERT_BACKLOG_LIMIT) | \
	 (1U << KALERT_PACKLOSS_COUNT))

int kalert_request_status(int fd, uint64_t mask)
{
	struct kalert_message req;

	memset(&req, 0, sizeof(req));
	req.nlh.nlmsg_len = NLMSG_LENGTH(0);
	req.nlh.nlmsg_type = KALERT_CMD_GET_STATUS;

	mnl_attr_put_u64(&req.nlh, KALERT_GET_STAT_MASK, mask);

	int rc = kalert_send_request(fd, &req);
	if (rc < 0)
		kalert_msg(LOG_WARNING, "Error sending status request (%s)",
			   strerror(-rc));
	return rc;
}

/**
 * kalert_set_parameter - Set a configuration parameter for the kalert channel
 * @attr_mask: The attribute type mask to specify attr to set(see enum kalert_chnl_attr_t).
 * @attr:     Attr value arry to set. The expected type depends on
 *            attr type(use 64-bit unsigned integer values to store).
 *
 * This function builds and sends a netlink message to the kalert subsystem
 * in order to configure a specific channel parameter. Supported attributes
 * include enabling/disabling the framework, setting the channel portid,
 * filter level, backlog limit, and clear packet loss count.
 *
 * Return:
 *   0 on success,
 *   a negative error code from on failure.
 */
int kalert_set_parameter(int fd, uint32_t attr_mask, uint64_t *attr)
{
	struct kalert_message req;
	int rc;
	int i;

	if (fd < 0)
		return -EBADF;

	if (attr_mask & ~ATTR_MASK_ALLOWED_SET) {
		kalert_msg(LOG_WARNING,
			   "Parameters to set are not allowed, mask:%x",
			   attr_mask);
		return -EINVAL;
	}

	memset(&req, 0, sizeof(req));
	req.nlh.nlmsg_len = NLMSG_LENGTH(0);
	req.nlh.nlmsg_type = KALERT_CMD_SET_CHNL;

	for (i = 1; i < KALERT_ATTR_MAX; i++) {
		if (attr_mask & KALERT_MASK(i)) {
			mnl_attr_put_u32(&req.nlh, i, attr[i]);
		}
	}

	rc = kalert_send_request(fd, &req);
	if (rc < 0) {
		kalert_msg(LOG_WARNING,
			   "Error setting kalert channel parameter (%s)",
			   strerror(-rc));
		return rc;
	}

	return 0;
}

int kalert_set_portid(int fd, uint32_t portid)
{
	uint64_t attr[KALERT_ATTR_MAX];

	if (portid <= 0) {
		kalert_msg(LOG_WARNING, "Try to set an invalid portid: %d",
			   portid);
		return -EINVAL;
	}
	attr[KALERT_PORTID] = portid;
	return kalert_set_parameter(fd, KALERT_MASK(KALERT_PORTID), attr);
}

int kalert_set_filter_level(int fd, uint32_t filter_level)
{
	uint64_t attr[KALERT_ATTR_MAX];

	if (filter_level >= KALERT_LEVEL_MAX) {
		kalert_msg(LOG_WARNING,
			   "Try to set an invalid filter level: %d",
			   filter_level);
		return -EINVAL;
	}

	attr[KALERT_FILTER_LEVEL] = filter_level;
	return kalert_set_parameter(fd, KALERT_MASK(KALERT_FILTER_LEVEL), attr);
}

int kalert_set_enable(int fd, uint32_t enable)
{
	uint64_t attr[KALERT_ATTR_MAX];

	if (enable != 0 && enable != 1)
		return -EINVAL;

	attr[KALERT_ENABLE] = enable;
	return kalert_set_parameter(fd, KALERT_MASK(KALERT_ENABLE), attr);
}

/**
 * kalert_start_channel - Open and initialize a kalert netlink channel
 *
 * This function opens a netlink socket to the kalert kernel subsystem
 * and sends an initialization request to enable the framework and
 * register the current process as the receiver of kalert notifications.
 * The default filter level is set to KALERT_WARN.
 *
 * Return:
 *   On success, returns the file descriptor of the opened kalert
 *   netlink socket.
 *   On failure, returns a negative error code.
 */
int kalert_start_channel(void)
{
	uint64_t attr[KALERT_ATTR_MAX];
	int sock_fd;
	int rc;

	if ((sock_fd = kalert_open()) < 0) {
		fprintf(stderr, "Cannot open netlink kalert socket\n");
		return -EBADF;
	}

	attr[KALERT_ENABLE] = 1;
	attr[KALERT_PORTID] = getpid();
	attr[KALERT_FILTER_LEVEL] = KALERT_WARN;
	rc = kalert_set_parameter(sock_fd,
				  KALERT_MASK(KALERT_ENABLE) |
					  KALERT_MASK(KALERT_PORTID) |
					  KALERT_MASK(KALERT_FILTER_LEVEL),
				  attr);
	if (rc < 0) {
		kalert_msg(LOG_WARNING,
			   "Error to start and set kalert channel(%s)",
			   strerror(-rc));
		return rc;
	}
	return sock_fd;
}

int kalert_subscribe_type(int fd, uint64_t type_mask, uint32_t level)
{
	struct kalert_message req;
	int rc;

	if (!TYPE_MASK_VALID(type_mask) || level >= KALERT_LEVEL_MAX)
		return -EINVAL;

	memset(&req, 0, sizeof(req));
	req.nlh.nlmsg_len = NLMSG_LENGTH(0);
	req.nlh.nlmsg_type = KALERT_CMD_SUBSCRIBE;

	mnl_attr_put_u64(&req.nlh, KALERT_SUB_TYPE_MASK, type_mask);
	mnl_attr_put_u32(&req.nlh, KALERT_SUB_LEVEL, level);

	rc = kalert_send_request(fd, &req);
	if (rc < 0) {
		kalert_msg(LOG_WARNING, "Error sending subscribe request (%s)",
			   strerror(-rc));
	}

	return rc;
}

int __kalert_subscribe_event(int fd, const int *event_ids, size_t count,
			     uint32_t level)
{
	struct kalert_message req;
	unsigned long event_mask[BITS_TO_LONGS(KALERT_EVENT_MAX)];
	int rc;

	if (!event_ids || count == 0)
		return -EINVAL;

	rc = events_to_bitmap(event_ids, count, event_mask, KALERT_EVENT_MAX);
	if (rc < 0)
		return rc;

	memset(&req, 0, sizeof(req));
	req.nlh.nlmsg_len = NLMSG_LENGTH(0);
	req.nlh.nlmsg_type = KALERT_CMD_SUBSCRIBE;

	mnl_attr_put_u32(&req.nlh, KALERT_SUB_LEVEL, level);
	mnl_attr_put(&req.nlh, KALERT_SUB_EVENT_MASK, sizeof(event_mask),
		     event_mask);

	rc = kalert_send_request(fd, &req);
	if (rc < 0) {
		kalert_msg(LOG_WARNING,
			   "Error sending event subscribe request (%d: %s)",
			   -rc, strerror(-rc));
	}

	return rc;
}
