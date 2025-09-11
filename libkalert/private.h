// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (C) 2025 Huiwen He <hehuiwen@kylinos.cn>
 *
 * Description: Internal definitions for libkalert
 */

#ifndef __PRIVATE_H_
#define __PRIVATE_H_

#include <libkalert/libkalert.h>
#include <limits.h>

#define TYPE_MASK_VALID(mask) \
	(((mask) != 0) &&     \
	 (((mask) & (((uint64_t)1 << (KALERT_NOTIFY_MAX + 1)) - 1)) != 0))

#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define BITS_TO_LONGS(nr) (((nr) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#define SET_BIT(bitmap, id) \
	((bitmap)[(id) / BITS_PER_LONG] |= (1UL << ((id) % BITS_PER_LONG)))

static inline int events_to_bitmap(const int *event_ids, size_t count,
				   unsigned long *bitmap, size_t nbits)
{
	if (!event_ids || !bitmap)
		return -EINVAL;

	size_t longs = BITS_TO_LONGS(nbits);
	for (size_t i = 0; i < longs; i++)
		bitmap[i] = 0;

	for (size_t i = 0; i < count; i++) {
		int index = event_ids[i] - KALERT_NOTIFY_BASE;
		if (index < 0 || (size_t)index >= nbits)
			return -EINVAL;

		SET_BIT(bitmap, index);
	}

	return 0;
}
#endif /* __PRIVATE_H */
