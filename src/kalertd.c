// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Main daemon for Kalert
 */

#include <stdio.h>
#include <libkalert/libkalert.h>

static int sock_fd;
static int msg_count;

void parse_notify_message(struct kalert_message *reply)
{
	struct kalert_notify_msg *notify;

	notify = NLMSG_DATA(&reply->nlh);

	switch (notify->type) {
	case KALERT_NOTIFY_MEM:
	case KALERT_NOTIFY_GEN:
	case KALERT_NOTIFY_FS: {
		printf("Received message %d,type = %d,level = %d, event=%d\n",
		       msg_count++, notify->type, notify->level, notify->event);
		break;
	}
	default:
		printf("Received message unknow\n");
	}
}

int main()
{
	int rc;
	struct kalert_message reply;

	printf("Kernel Fault Events Alert daemon starting...\n");
	kalert_msg(LOG_INFO, "Kalert daemon starting...");

	sock_fd = kalert_start_channel();
	if (sock_fd < 0) {
		printf("Failed to initialize alert subsystem, exiting...\n");
		kalert_msg(LOG_INFO,
			   "Kalert daemon starting failed, exiting...");
		return -1;
	}

	while (1) {
		rc = kalert_get_reply(sock_fd, &reply, GET_REPLY_BLOCKING, 0);
		if (rc > 0)
			parse_notify_message(&reply);
	}
	return 0;
}
