/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_JIKAN_H
#define ANI_JIKAN_H

#include "ani/models.h"
#include <stdbool.h>

// Search for anime by query and populate series info
bool ani_jikan_search_anime(const char *query, ani_series *series);

#endif // ANI_JIKAN_H
