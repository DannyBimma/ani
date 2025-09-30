/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_FS_H
#define ANI_FS_H

#include <stdbool.h>

// Get platform-specific cache directory
char *ani_get_cache_dir(void);

// Create directory (including parents if needed)
bool ani_mkdir_p(const char *path);

// Join path components
char *ani_path_join(const char *base, const char *name);

#endif // ANI_FS_H
