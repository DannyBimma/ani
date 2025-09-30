/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_MANGADEX_H
#define ANI_MANGADEX_H

#include "ani/models.h"
#include <stdbool.h>

// Search for manga by query and populate series info
bool ani_mangadex_search_manga(const char *query, ani_series *series);

// Get latest chapter for a manga ID
bool ani_mangadex_get_latest_chapter(const char *manga_id, ani_series *series);

#endif // ANI_MANGADEX_H
