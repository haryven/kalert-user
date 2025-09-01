// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Public API for libkalert
 */

#ifndef LIBKALERT_H
#define LIBKALERT_H
#include <errno.h>
#include <linux/kalert.h>
#include <linux/netlink.h>
#include <string.h>
#include <sys/poll.h>
#include <syslog.h>

#define kalert_msg(priority, format, ...) \
	syslog(priority, format, ##__VA_ARGS__)

#define KALERT_MAX_MSG_SIZE 8192 // MNL_SOCKET_BUFFER_SIZE
#define KALERT_MASK(type) (1U << (type))

struct kalert_message {
	struct nlmsghdr nlh;
	char data[KALERT_MAX_MSG_SIZE];
};

/* kalert channel attribute -> string map */
static const char *const kalert_chnl_attr_to_str[] = {
	[KALERT_ATTR_UNSPEC] = "unknow",
	[KALERT_ENABLE] = "enable",
	[KALERT_PORTID] = "portid",
	[KALERT_FILTER_LEVEL] = "filter_level",
	[KALERT_BACKLOG_LIMIT] = "backlog_limit",
	[KALERT_BACKLOG_DEPTH] = "backlog_depth",
	[KALERT_PACKLOSS_COUNT] = "packloss_count",
	[KALERT_GET_STAT_MASK] = "get_stat_mask",
};

typedef enum { GET_REPLY_BLOCKING = 0, GET_REPLY_NONBLOCKING } reply_t;

/* Base interface */
int kalert_open(void);
void kalert_close(int fd);
int kalert_send_request(int fd, struct kalert_message *req);
int kalert_get_reply(int fd, struct kalert_message *rep, reply_t block,
		     int peek);

/* Advance wrap interface */
int kalert_start_channel(void);

#endif /* LIBKALERT_H */
