// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Shared utility declarations used across
 * kalert user-space modules.
 *
 */

#ifndef KALERT_COMMON_H_
#define KALERT_COMMON_H_

#include <stdbool.h>
#include <stdlib.h> /* atoi() */

/*
 * parse_config()
 *
 * Reads a configuration file line-by-line, trims whitespace, skips comments
 * and blank lines, and invokes parse_line() for each valid entry.
 *
 * parse_line():
 *   Return true if the line is processed successfully.
 *   Return false to indicate a parse failure (but parsing continues).
 */
bool parse_config(const char *conf,
		  bool (*parse_line)(const char *key, const char *val));

#endif /* KALERT_COMMON_H_ */
