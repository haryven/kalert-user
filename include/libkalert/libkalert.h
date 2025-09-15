// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Public API for libkalert
 */

#ifndef LIBKALERT_H
#define LIBKALERT_H
#include <syslog.h>
#include <linux/netlink.h>
#include <linux/kalert.h>

#define kalert_msg(priority, format, ...) \
	syslog(priority, format, ##__VA_ARGS__)

#define KALERT_MAX_MSG_SIZE 8192 // MNL_SOCKET_BUFFER_SIZE

struct kalert_message {
	struct nlmsghdr nlh;
	char data[KALERT_MAX_MSG_SIZE];
};

typedef enum { GET_REPLY_BLOCKING = 0, GET_REPLY_NONBLOCKING } reply_t;

/* Base interface */
int kalert_open(void);
void kalert_close(int fd);
int kalert_send_request(int fd, struct kalert_message *req);
int kalert_get_reply(int fd, struct kalert_message *rep, reply_t block,
		     int peek);

#endif /* LIBKALERT_H */
