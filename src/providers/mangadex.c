/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/providers/mangadex.h"
#include "ani/http.h"
#include "ani/json.h"
#include "ani/log.h"
#include "ani/str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MANGADEX_BASE_URL "https://api.mangadex.org"
#define MANGADEX_SEARCH_URL                                                    \
  MANGADEX_BASE_URL "/manga?title=%s&limit=1&order[relevance]=desc"
#define MANGADEX_CHAPTER_URL                                                   \
  MANGADEX_BASE_URL "/chapter?manga=%s&translatedLanguage[]=en&limit=1&order[" \
                    "publishAt]=desc"
#define MANGADEX_AGGREGATE_URL                                                 \
  MANGADEX_BASE_URL "/manga/%s/aggregate?translatedLanguage[]=en"

// URL encode helper (same as jikan.c)
static char *url_encode(const char *str) {
  size_t len;
  size_t i;
  size_t j;
  char *encoded;
  const char *hex = "0123456789ABCDEF";

  if (str == NULL) {
    return NULL;
  }

  len = strlen(str);
  encoded = malloc(len * 3 + 1);
  if (encoded == NULL) {
    return NULL;
  }

  j = 0;
  for (i = 0; i < len; i++) {
    unsigned char c = (unsigned char)str[i];

    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' ||
        c == '~') {
      encoded[j++] = (char)c;
    } else if (c == ' ') {
      encoded[j++] = '+';
    } else {
      encoded[j++] = '%';
      encoded[j++] = hex[(c >> 4) & 0xF];
      encoded[j++] = hex[c & 0xF];
    }
  }
  encoded[j] = '\0';

  return encoded;
}

// Parse manga titles from JSON
static void parse_titles(ani_json_val *manga_obj, ani_series *series) {
  ani_json_val *attributes;
  ani_json_val *title_obj;
  ani_json_val *alt_titles;
  const char *english;
  const char *japanese;
  const char *canonical;
  size_t i;

  if (manga_obj == NULL || series == NULL) {
    return;
  }

  attributes = ani_json_object_get(manga_obj, "attributes");
  if (attributes == NULL) {
    return;
  }

  english = NULL;
  japanese = NULL;
  canonical = NULL;

  // Get main title
  title_obj = ani_json_object_get(attributes, "title");
  if (title_obj != NULL) {
    // Try en first
    canonical = ani_json_object_get_string(title_obj, "en");
    if (canonical == NULL) {
      // Try ja-ro (romaji)
      canonical = ani_json_object_get_string(title_obj, "ja-ro");
    }
    if (canonical == NULL) {
      // Try ja
      canonical = ani_json_object_get_string(title_obj, "ja");
    }
  }

  // Search altTitles for English/Japanese
  alt_titles = ani_json_object_get(attributes, "altTitles");
  if (alt_titles != NULL && ani_json_is_array(alt_titles)) {
    size_t count = ani_json_array_size(alt_titles);
    for (i = 0; i < count; i++) {
      ani_json_val *alt_title = ani_json_array_get(alt_titles, i);
      if (alt_title == NULL) {
        continue;
      }

      if (english == NULL) {
        english = ani_json_object_get_string(alt_title, "en");
      }
      if (japanese == NULL) {
        japanese = ani_json_object_get_string(alt_title, "ja");
      }
    }
  }

  // Fallback: use canonical as English if not found
  if (english == NULL) {
    english = canonical;
  }

  ani_title_set(&series->title, english, japanese, canonical);
}

bool ani_mangadex_search_manga(const char *query, ani_series *series) {
  char url[512];
  char *encoded_query;
  ani_http_response *resp;
  ani_json_doc *doc;
  ani_json_val *root;
  ani_json_val *data_array;
  ani_json_val *manga_obj;
  ani_json_val *id_val;
  bool success;

  if (query == NULL || series == NULL) {
    return false;
  }

  // URL encode query
  encoded_query = url_encode(query);
  if (encoded_query == NULL) {
    return false;
  }

  // Build search URL
  snprintf(url, sizeof(url), MANGADEX_SEARCH_URL, encoded_query);
  free(encoded_query);

  LOG_DEBUG("MangaDex search: %s", url);

  // Make HTTP request
  resp = ani_http_get(url, NULL);
  if (resp == NULL || resp->status_code != 200) {
    LOG_ERROR("MangaDex search failed: HTTP %ld", resp ? resp->status_code : 0);
    ani_http_response_free(resp);
    return false;
  }

  // Parse JSON
  doc = ani_json_parse(resp->body, resp->body_len);
  ani_http_response_free(resp);

  if (doc == NULL) {
    return false;
  }

  success = false;
  root = ani_json_get_root(doc);
  if (root != NULL) {
    data_array = ani_json_object_get(root, "data");
    if (data_array != NULL && ani_json_array_size(data_array) > 0) {
      manga_obj = ani_json_array_get(data_array, 0);
      if (manga_obj != NULL) {
        // Get manga ID
        id_val = ani_json_object_get(manga_obj, "id");
        if (id_val != NULL) {
          const char *id = ani_json_get_string(id_val);
          if (id != NULL) {
            series->id = ani_strdup(id);

            // Parse titles
            parse_titles(manga_obj, series);

            series->media_type = ANI_MEDIA_MANGA;
            series->provider = ani_strdup("mangadex");
            success = true;

            LOG_INFO("Found manga: %s (ID: %s)",
                     series->title.canonical ? series->title.canonical
                                             : "unknown",
                     series->id);
          }
        }
      }
    }
  }

  ani_json_doc_free(doc);
  return success;
}

bool ani_mangadex_get_latest_chapter(const char *manga_id, ani_series *series) {
  char url[512];
  ani_http_response *resp;
  ani_json_doc *doc;
  ani_json_val *root;
  ani_json_val *data_array;
  ani_json_val *chapter_obj;
  ani_json_val *attributes;
  const char *chapter_num_str;
  const char *publish_date_str;
  bool success;

  if (manga_id == NULL || series == NULL) {
    return false;
  }

  // Build chapter URL
  snprintf(url, sizeof(url), MANGADEX_CHAPTER_URL, manga_id);

  LOG_DEBUG("MangaDex latest chapter: %s", url);

  // Make HTTP request
  resp = ani_http_get(url, NULL);
  if (resp == NULL || resp->status_code != 200) {
    LOG_WARN("MangaDex chapter query failed: HTTP %ld",
             resp ? resp->status_code : 0);
    ani_http_response_free(resp);
    return false;
  }

  // Parse JSON
  doc = ani_json_parse(resp->body, resp->body_len);
  ani_http_response_free(resp);

  if (doc == NULL) {
    return false;
  }

  success = false;
  root = ani_json_get_root(doc);
  if (root != NULL) {
    data_array = ani_json_object_get(root, "data");
    if (data_array != NULL && ani_json_array_size(data_array) > 0) {
      chapter_obj = ani_json_array_get(data_array, 0);
      if (chapter_obj != NULL) {
        attributes = ani_json_object_get(chapter_obj, "attributes");
        if (attributes != NULL) {
          // Get chapter number
          chapter_num_str = ani_json_object_get_string(attributes, "chapter");
          if (chapter_num_str != NULL) {
            series->release.latest_number = atoi(chapter_num_str);
          }

          // Get publish date
          publish_date_str =
              ani_json_object_get_string(attributes, "publishAt");
          if (publish_date_str != NULL) {
            ani_parse_iso8601(publish_date_str, &series->release.latest_date);
          }

          success = true;
          LOG_DEBUG("Latest chapter: %d on %s", series->release.latest_number,
                    publish_date_str ? publish_date_str : "unknown");
        }
      }
    }
  }

  ani_json_doc_free(doc);
  return success;
}
