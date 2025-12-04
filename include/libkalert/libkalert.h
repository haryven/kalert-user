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
#include <stdint.h>
#include <string.h>
#include <sys/poll.h>
#include <syslog.h>

#define kalert_msg(priority, format, ...) \
	syslog(priority, format, ##__VA_ARGS__)

#ifndef KALERT_ARRAY_SIZE
#define KALERT_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

/**
 * @brief Subscribe to a set of event IDs.
 *
 * This macro automatically calculates the number of event IDs in the array
 * and calls kalert_subscribe_event_with_count().
 *
 * @param fd     Netlink socket file descriptor.
 * @param ids    Array of event IDs to subscribe.
 * @param level  Subscription level.
 *
 * Example:
 * @code
 * int ids[] = { KALERT_MEM_OOM, KALERT_MEM_LEAK };
 * kalert_subscribe_event(fd, ids, KALERT_LEVEL_WARN);
 * @endcode
 */
#define kalert_subscribe_event(fd, ids, level) \
	__kalert_subscribe_event(fd, ids, KALERT_ARRAY_SIZE(ids), level)

#define KALERT_MAX_MSG_SIZE 8192 // MNL_SOCKET_BUFFER_SIZE
#define KALERT_MASK(type) (1U << (type))

struct kalert_message {
	struct nlmsghdr nlh;
	char data[KALERT_MAX_MSG_SIZE];
};

// clang-format off
/* kalert channel attribute -> string map */
static const char* const kalert_chnl_attr_to_str[] = {
	[KALERT_ATTR_UNSPEC] = "unknow",
	[KALERT_ENABLE] = "enable",
	[KALERT_PORTID] = "portid",
	[KALERT_FILTER_LEVEL] = "filter_level",
	[KALERT_BACKLOG_LIMIT] = "backlog_limit",
	[KALERT_BACKLOG_DEPTH] = "backlog_depth",
	[KALERT_PACKLOSS_COUNT] = "packloss_count",
	[KALERT_GET_STAT_MASK] = "get_stat_mask"
};

static const char* const kalert_type_to_str[] = {
	[KALERT_NOTIFY_ALL] = "unknow",
	[KALERT_NOTIFY_GEN] = "generic",
	[KALERT_NOTIFY_MEM] = "mem",
	[KALERT_NOTIFY_IO] = "io",
	[KALERT_NOTIFY_FS] = "fs",
	[KALERT_NOTIFY_SCHED] = "sched",
	[KALERT_NOTIFY_NET] = "net",
	[KALERT_NOTIFY_RAS] = "ras",
	[KALERT_NOTIFY_VIRT] = "virtual",
	[KALERT_NOTIFY_SEC] = "security"
};

static const char* const kalert_level_to_str[] = {
	[KALERT_LEVEL_ALL] = "unknow",
	[KALERT_INFO] = "info",
	[KALERT_WARN] = "warn",
	[KALERT_ERROR] = "error",
	[KALERT_FATAL] = "fatal"
};

static const char * const kalert_event_str[] = {
	[KALERT_GEN_SOFTLOCKUP - KALERT_EVENT_BASE] = "softlockup",
	[KALERT_GEN_RCUSTALL   - KALERT_EVENT_BASE] = "rcu stall",
	[KALERT_GEN_HUNGTASK   - KALERT_EVENT_BASE] = "hung task",
	[KALERT_MEM_ALLOCFAIL  - KALERT_EVENT_BASE] = "mem alloc fail",
	[KALERT_MEM_OOM        - KALERT_EVENT_BASE] = "oom",
	[KALERT_MEM_BAD_STATE  - KALERT_EVENT_BASE] = "bad mem state",
	[KALERT_MEM_LEAK       - KALERT_EVENT_BASE] = "mem leak",
	[KALERT_FS_EXT4_ERR    - KALERT_EVENT_BASE] = "ext4 err",
};
// clang-format on

static inline const char *kalert_event_name(int eventid)
{
	int idx = eventid - KALERT_EVENT_BASE;
	if (idx < 0 || (size_t)idx >= KALERT_ARRAY_SIZE(kalert_event_str))
		return "unknown";
	return kalert_event_str[idx] ?: "unknown";
}

typedef enum { GET_REPLY_BLOCKING = 0, GET_REPLY_NONBLOCKING } reply_t;

/* Base interface */
int kalert_open(void);
void kalert_close(int fd);
int kalert_send_request(int fd, struct kalert_message *req);
int kalert_get_reply(int fd, struct kalert_message *rep, reply_t block,
		     int peek);

/* Advance wrap interface */
int kalert_start_channel(void);
int __kalert_subscribe_event(int fd, const int *event_ids, size_t count,
			     uint32_t level);
int kalert_subscribe_type(int fd, uint64_t type_mask, uint32_t level);

#endif /* LIBKALERT_H */
