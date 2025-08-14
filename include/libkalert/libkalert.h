// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Public API for libkalert
 */

#ifndef LIBKALERT_H
#define LIBKALERT_H
#include <syslog.h>

#define kalert_msg(priority, format, ...) \
	syslog(priority, format, ##__VA_ARGS__)

#endif /* LIBKALERT_H */
