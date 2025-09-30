/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_CACHE_H
#define ANI_CACHE_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

/* Cache entry TTLs (in seconds) */
#define ANI_CACHE_TTL_SEARCH   300    /* 5 minutes */
#define ANI_CACHE_TTL_DETAILS  21600  /* 6 hours */
#define ANI_CACHE_TTL_SCHEDULE 1800   /* 30 minutes */

/* Initialize cache directory */
bool ani_cache_init(void);

/* Get cached data if valid */
char *ani_cache_get(const char *provider, const char *key, time_t max_age);

/* Store data in cache */
bool ani_cache_set(const char *provider, const char *key, const char *data);

/* Clear all cache */
void ani_cache_clear(void);

#endif /* ANI_CACHE_H */
