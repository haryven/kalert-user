// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Main daemon for Kalert
 */

#include <stdio.h>
#include <libkalert/libkalert.h>

int main()
{
	printf("Kernel Fault Events Alert daemon starting...\n");
	kalert_msg(LOG_INFO, "Kalert daemon starting...");
	return 0;
}
