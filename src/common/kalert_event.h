// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Kernel fault event logging library for kalertd
 * and user-space subscriber processes.
 *
 * Notes:
 *    Thread Safety: NOT thread-safe. Only one thread per process should
 *    write events.
 */

#ifndef KALERT_EVENT_H
#define KALERT_EVENT_H

#include <stdio.h>

/**
 * kalert_event_log_init - Initialize event logging to a file
 * @path: Path to log file. If NULL, defaults to KALERT_EVENT_FILE
 *
 * Returns 0 on success, -1 on failure.
 */
int kalert_event_log_init(const char *path);

/**
 * kalert_event_set_utc - Set timestamp mode to UTC or local time
 * @flag: 1 to use UTC, 0 to use local time
 */
void kalert_event_set_utc(int flag);

/**
 * kalert_event - Write formatted event message with timestamp
 * @fmt: printf-style format string
 * @...: arguments
 *
 * Flushes after each write. NOT thread-safe.
 */
void kalert_event(const char *fmt, ...);

/**
 * kalert_event_log_close - Close the event log file
 */
void kalert_event_log_close(void);

#endif /* KALERT_EVENT_H */
