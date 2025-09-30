/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/cache.h"
#include "ani/fs.h"
#include "ani/log.h"
#include "ani/str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static char *cache_dir = NULL;

bool ani_cache_init(void) {
  if (cache_dir != NULL) {
    return true; /* Already initialized */
  }

  cache_dir = ani_get_cache_dir();
  if (cache_dir == NULL) {
    LOG_WARN("Failed to get cache directory");
    return false;
  }

  /* Create cache directory if it doesn't exist */
  if (!ani_mkdir_p(cache_dir)) {
    LOG_WARN("Failed to create cache directory: %s", cache_dir);
    free(cache_dir);
    cache_dir = NULL;
    return false;
  }

  LOG_DEBUG("Cache initialized: %s", cache_dir);
  return true;
}

static char *get_cache_path(const char *provider, const char *key) {
  char filename[512];
  char *path;

  if (cache_dir == NULL || provider == NULL || key == NULL) {
    return NULL;
  }

  /* Simple filename: provider_key.json */
  snprintf(filename, sizeof(filename), "%s_%s.json", provider, key);

  path = ani_path_join(cache_dir, filename);
  return path;
}

char *ani_cache_get(const char *provider, const char *key, time_t max_age) {
  char *path;
  struct stat st;
  FILE *f;
  char *data;
  size_t file_size;
  size_t read_size;
  time_t now;
  time_t age;

  path = get_cache_path(provider, key);
  if (path == NULL) {
    return NULL;
  }

  /* Check if file exists and get stats */
  if (stat(path, &st) != 0) {
    free(path);
    return NULL; /* File doesn't exist */
  }

  /* Check age */
  now = time(NULL);
  age = now - st.st_mtime;
  if (age > max_age) {
    LOG_DEBUG("Cache expired for %s/%s (age: %ld sec)", provider, key, age);
    free(path);
    return NULL;
  }

  /* Read file */
  f = fopen(path, "r");
  free(path);

  if (f == NULL) {
    return NULL;
  }

  file_size = (size_t)st.st_size;
  data = malloc(file_size + 1);
  if (data == NULL) {
    fclose(f);
    return NULL;
  }

  read_size = fread(data, 1, file_size, f);
  fclose(f);

  if (read_size != file_size) {
    free(data);
    return NULL;
  }

  data[file_size] = '\0';
  LOG_DEBUG("Cache hit for %s/%s", provider, key);
  return data;
}

bool ani_cache_set(const char *provider, const char *key, const char *data) {
  char *path;
  FILE *f;
  size_t data_len;
  size_t written;

  if (data == NULL) {
    return false;
  }

  path = get_cache_path(provider, key);
  if (path == NULL) {
    return false;
  }

  f = fopen(path, "w");
  free(path);

  if (f == NULL) {
    return false;
  }

  data_len = strlen(data);
  written = fwrite(data, 1, data_len, f);
  fclose(f);

  if (written != data_len) {
    return false;
  }

  LOG_DEBUG("Cached %s/%s (%zu bytes)", provider, key, data_len);
  return true;
}

void ani_cache_clear(void) {
  /* TODO: Implement cache clearing */
  LOG_INFO("Cache clear not yet implemented");
}
