// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Main daemon for Kalert
 */

#include <stdio.h>
#include <libkalert/libkalert.h>
#include <ev.h>

#include "common/kalert_event.h"
#include "common/common.h"

static int sock_fd;
static int msg_count;

static struct ev_loop *loop;
static struct ev_io netlink_watcher;
static struct ev_signal sigterm_watcher;
static struct ev_signal sighup_watcher;

/* Global variables */
bool g_flag_utc = false;

#define KALERT_EVENT_LOG_FILE "/var/log/kalert_event.log"
#define KALERTD_CONF_FILE "/etc/kalert/kalertd.conf"

bool parse_main_conf_line(const char *key, const char *val)
{
	if (strcmp(key, "UTC_TIME") == 0) {
		g_flag_utc = (strcasecmp(val, "on") == 0);
		return true;
	}
	return false;
}

/* Load kalertd configuration safely into global g_cfg */
bool load_kalertd_config(void)
{
	if (!parse_config(KALERTD_CONF_FILE, parse_main_conf_line)) {
		kalert_msg(LOG_ERR,
			   "Failed to parse config, using previous values\n");
		return false;
	}

	kalert_event_set_utc(g_flag_utc);

	return true;
}

void parse_notify_message(struct kalert_message *reply)
{
	struct kalert_notify_msg *notify;
	const char *type_str, *level_str, *event_str;

	notify = NLMSG_DATA(&reply->nlh);
	if (!kalert_notify_valid(notify))
		return;

	type_str = kalert_type_str[notify->type];
	level_str = kalert_level_str[notify->level];
	event_str = kalert_event_name(notify->event);

	switch (notify->type) {
	case KALERT_NOTIFY_MEM:
	case KALERT_NOTIFY_GEN:
	case KALERT_NOTIFY_IO:
	case KALERT_NOTIFY_FS:
	case KALERT_NOTIFY_SCHED:
	case KALERT_NOTIFY_NET:
	case KALERT_NOTIFY_RAS:
	case KALERT_NOTIFY_VIRT:
	case KALERT_NOTIFY_SEC: {
		kalert_event(
			"{\"ts\":%llu,\"type\":%s,\"event\":%s,\"level\":%s}\n",
			msg_count++, type_str, event_str, level_str);
		break;
	}
	case KALERT_NOTIFY_ALL:
		break;
	default:
		kalert_event(
			"{\"ts\":%llu,\"type\":\"unknow\",\"event\":%s,\"level\":%s\n",
			msg_count++, event_str, level_str);
	}
}

/* ---------------------- Netlink event Handler ----------------- */
static void netlink_handler(struct ev_loop *loop, struct ev_io *w, int revents)
{
	struct kalert_message reply;

	while (kalert_get_reply(sock_fd, &reply, GET_REPLY_NONBLOCKING, 0) > 0)
		parse_notify_message(&reply);
}

/* ---------------------- Reload Config Handler ----------------- */
static void hup_handler(struct ev_loop *loop, struct ev_signal *w, int revents)
{
	kalert_msg(LOG_INFO, "Received SIGHUP, reloading configuration...");
	if (!load_kalertd_config())
		kalert_msg(LOG_WARNING, "Reload configuration failed \n");
}

/* ---------------------- Termination Handler ------------------- */
static void term_handler(struct ev_loop *loop, struct ev_signal *w, int revents)
{
	kalert_msg(LOG_INFO, "Received termination signal, shutting down...");
	ev_io_stop(loop, &netlink_watcher);
	ev_signal_stop(loop, &sigterm_watcher);
	ev_signal_stop(loop, &sighup_watcher);
	ev_break(loop, EVBREAK_ALL);
}

static void start_event_loop()
{
	loop = EV_DEFAULT;
	/* Register netlink event watcher */
	ev_io_init(&netlink_watcher, netlink_handler, sock_fd, EV_READ);
	ev_io_start(loop, &netlink_watcher);

	/* Register signal handlers */
	ev_signal_init(&sigterm_watcher, term_handler, SIGTERM);
	ev_signal_start(loop, &sigterm_watcher);

	ev_signal_init(&sighup_watcher, hup_handler, SIGHUP);
	ev_signal_start(loop, &sighup_watcher);

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
		kalert_msg(LOG_ERR,
			   "Kalert daemon starting failed, exiting...");
		return -1;
	}

	if (!load_kalertd_config())
		return -1;

	if (kalert_event_log_init(KALERT_EVENT_LOG_FILE))
		kalert_msg(LOG_WARNING,
			   "Failed to open kalert events log file");

	start_event_loop();

	kalert_close(sock_fd);

	return 0;
}
