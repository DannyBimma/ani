/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/fs.h"
#include "ani/str.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#endif

char *
ani_get_cache_dir(void)
{
	const char *base;
	char *cache_dir;

#ifdef _WIN32
	/* Windows: %LOCALAPPDATA%\ani\Cache */
	base = getenv("LOCALAPPDATA");
	if (base == NULL) {
		base = getenv("APPDATA");
	}
	if (base == NULL) {
		return NULL;
	}
	cache_dir = ani_path_join(base, "ani");
	if (cache_dir != NULL) {
		char *full_path = ani_path_join(cache_dir, "Cache");
		free(cache_dir);
		return full_path;
	}
	return NULL;
#elif defined(__APPLE__)
	/* macOS: ~/Library/Caches/ani */
	base = getenv("HOME");
	if (base == NULL) {
		return NULL;
	}
	cache_dir = ani_path_join(base, "Library");
	if (cache_dir != NULL) {
		char *caches = ani_path_join(cache_dir, "Caches");
		free(cache_dir);
		if (caches != NULL) {
			char *full_path = ani_path_join(caches, "ani");
			free(caches);
			return full_path;
		}
	}
	return NULL;
#else
	/* Linux: $XDG_CACHE_HOME/ani or ~/.cache/ani */
	base = getenv("XDG_CACHE_HOME");
	if (base != NULL && base[0] != '\0') {
		return ani_path_join(base, "ani");
	}

	base = getenv("HOME");
	if (base == NULL) {
		return NULL;
	}
	cache_dir = ani_path_join(base, ".cache");
	if (cache_dir != NULL) {
		char *full_path = ani_path_join(cache_dir, "ani");
		free(cache_dir);
		return full_path;
	}
	return NULL;
#endif
}

bool
ani_mkdir_p(const char *path)
{
	char *copy;
	char *p;
	struct stat st;

	if (path == NULL || path[0] == '\0') {
		return false;
	}

	/* Check if already exists */
	if (stat(path, &st) == 0) {
		return S_ISDIR(st.st_mode);
	}

	/* Create a mutable copy */
	copy = ani_strdup(path);
	if (copy == NULL) {
		return false;
	}

	/* Create parent directories */
	p = copy;
	if (*p == '/') {
		p++;
	}

	while (*p != '\0') {
		if (*p == '/' || *p == '\\') {
			*p = '\0';

			if (stat(copy, &st) != 0) {
				if (mkdir(copy, 0755) != 0 && errno != EEXIST) {
					free(copy);
					return false;
				}
			} else if (!S_ISDIR(st.st_mode)) {
				free(copy);
				return false;
			}

			*p = '/';
		}
		p++;
	}

	/* Create final directory */
	if (mkdir(copy, 0755) != 0 && errno != EEXIST) {
		free(copy);
		return false;
	}

	free(copy);
	return true;
}

bool
ani_file_exists(const char *path)
{
	struct stat st;
	return stat(path, &st) == 0;
}

char *
ani_path_join(const char *base, const char *name)
{
	size_t base_len;
	size_t name_len;
	size_t total_len;
	char *result;
	bool needs_sep;

	if (base == NULL || name == NULL) {
		return NULL;
	}

	base_len = strlen(base);
	name_len = strlen(name);

	/* Check if we need a separator */
	needs_sep = base_len > 0 && base[base_len - 1] != '/' && base[base_len - 1] != '\\';

	total_len = base_len + (needs_sep ? 1 : 0) + name_len;
	result = malloc(total_len + 1);
	if (result == NULL) {
		return NULL;
	}

	memcpy(result, base, base_len);
	if (needs_sep) {
#ifdef _WIN32
		result[base_len] = '\\';
#else
		result[base_len] = '/';
#endif
		memcpy(result + base_len + 1, name, name_len);
	} else {
		memcpy(result + base_len, name, name_len);
	}

	result[total_len] = '\0';
	return result;
}
