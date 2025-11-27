// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Main daemon for Kalert
 */

#include <stdio.h>
#include <libkalert/libkalert.h>
#include <ev.h>

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

static void netlink_handler(struct ev_loop *loop, struct ev_io *w, int revents)
{
	struct kalert_message reply;

	while (kalert_get_reply(sock_fd, &reply, GET_REPLY_NONBLOCKING, 0) > 0)
		parse_notify_message(&reply);
}

static void start_event_loop()
{
	struct ev_loop *loop = EV_DEFAULT;
	struct ev_io netlink_watcher;

	/* Register netlink event watcher */
	ev_io_init(&netlink_watcher, netlink_handler, sock_fd, EV_READ);
	ev_io_start(loop, &netlink_watcher);

	/* Starting event loop */
	ev_run(loop, 0);

	ev_io_stop(loop, &netlink_watcher);
}

int main()
{
	printf("Kernel Fault Events Alert daemon starting...\n");
	kalert_msg(LOG_INFO, "Kalert daemon starting...");

	sock_fd = kalert_start_channel();
	if (sock_fd < 0) {
		printf("Failed to initialize alert subsystem, exiting...\n");
		kalert_msg(LOG_INFO,
			   "Kalert daemon starting failed, exiting...");
		return -1;
	}

	start_event_loop();

	kalert_close(sock_fd);

	return 0;
}
