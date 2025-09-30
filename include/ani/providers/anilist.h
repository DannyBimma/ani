/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_ANILIST_H
#define ANI_ANILIST_H

#include "ani/models.h"
#include <stdbool.h>

// Get next airing episode info using MAL ID
bool ani_anilist_get_next_episode(const char *mal_id, ani_series *series);

#endif // ANI_ANILIST_H
