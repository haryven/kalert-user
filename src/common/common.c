// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Shared utility declarations used across
 * kalert user-space modules.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "common.h"

#define MAX_LINE 256

/* Trim leading/trailing spaces */
static char *trim(char *s)
{
	while (isspace(*s))
		s++;
	char *end = s + strlen(s) - 1;
	while (end > s && isspace(*end))
		*end-- = '\0';
	return s;
}

/* Remove surrounding single/double quotes */
static void unquote(char *s)
{
	size_t len = strlen(s);
	if (len >= 2) {
		if ((s[0] == '"' && s[len - 1] == '"') ||
		    (s[0] == '\'' && s[len - 1] == '\'')) {
			s[len - 1] = '\0';
			memmove(s, s + 1, len - 1);
		}
	}
}

/*
 * parse_config:
 *   fp: config file
 *   parse_line: callback to handle KEY/value pair
 */
bool parse_config(const char *path,
		  bool (*parse_line)(const char *key, const char *val))
{
	char buf[MAX_LINE];
	FILE *fp = fopen(path, "r");
	if (!fp)
		return false;

	int parsed = 0;

	while (fgets(buf, sizeof(buf), fp)) {
		char *line = trim(buf);

		/* Skip empty or comment lines */
		if (*line == '\0' || *line == '#')
			continue;

		/* Split key = val */
		char *eq = strchr(line, '=');
		if (!eq)
			continue;

		*eq = '\0';
		char *key = trim(line);
		char *val = trim(eq + 1);
		unquote(val);

		/* Just ignore unknown config */
		if (!parse_line(key, val))
			continue;

		parsed++;
	}

	fclose(fp);

	/* No valid configuration lines */
	return parsed > 0;
}
