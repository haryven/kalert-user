// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: kalert events logging implementation
 */

#include "kalert_event.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

/* File pointer for the log file */
static FILE *fp = NULL;

/* Use UTC time or local time for timestamps */
static int use_utc = 0;

/* Cached seconds for timestamp optimization */
static time_t cached_sec = -1;

/* Cached formatted timestamp string */
static char cached_ts[64];

/* Internal: get timestamp string (cached per second) */
static const char *get_ts(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	/* Update cache only if seconds changed */
	if (ts.tv_sec != cached_sec) {
		cached_sec = ts.tv_sec;
		struct tm tm;

		if (use_utc)
			gmtime_r(&cached_sec, &tm);
		else
			localtime_r(&cached_sec, &tm);

		snprintf(cached_ts, sizeof(cached_ts),
			 "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
			 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			 tm.tm_hour, tm.tm_min, tm.tm_sec,
			 ts.tv_nsec / 1000000);
	}

	return cached_ts;
}

/* Initialize event logging */
int kalert_event_log_init(const char *path)
{
	if (fp)
		return 0;

	if (!path)
		return -1;

	fp = fopen(path, "a");
	if (!fp)
		return -1;
	return 0;
}

/* Set UTC or local time mode */
void kalert_event_set_utc(int flag)
{
	use_utc = flag;
}

/* Write event log message with timestamp */
void kalert_event(const char *fmt, ...)
{
	if (!fp)
		return;

	fprintf(fp, "%s ", get_ts());

	va_list ap;
	va_start(ap, fmt);
	vfprintf(fp, fmt, ap);
	va_end(ap);

	fflush(fp);
}

/* Close event log file */
void kalert_event_log_close(void)
{
	if (fp) {
		fclose(fp);
		fp = NULL;
	}
}
