/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/providers/jikan.h"
#include "ani/http.h"
#include "ani/json.h"
#include "ani/log.h"
#include "ani/str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JIKAN_BASE_URL "https://api.jikan.moe/v4"
#define JIKAN_SEARCH_URL                                                       \
  JIKAN_BASE_URL "/anime?q=%s&limit=1&order_by=popularity"
#define JIKAN_ANIME_URL JIKAN_BASE_URL "/anime/%s"

// URL encode a query string
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
  // Worst case: every char becomes %XX (3x expansion)
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

// Parse anime titles from JSON
static void parse_titles(ani_json_val *anime_obj, ani_series *series) {
  ani_json_val *title_obj;
  const char *english;
  const char *japanese;
  const char *canonical;

  if (anime_obj == NULL || series == NULL) {
    return;
  }

  // Try title object first
  title_obj = ani_json_object_get(anime_obj, "title");
  canonical = ani_json_get_string(title_obj);

  // Get English title
  title_obj = ani_json_object_get(anime_obj, "title_english");
  english = ani_json_get_string(title_obj);

  // Get Japanese title
  title_obj = ani_json_object_get(anime_obj, "title_japanese");
  japanese = ani_json_get_string(title_obj);

  // Set titles
  ani_title_set(&series->title, english, japanese, canonical);
}

// Parse episode count and aired dates
static void parse_details(ani_json_val *anime_obj, ani_series *series) {
  ani_json_val *val;
  ani_json_val *aired_obj;
  ani_json_val *from_obj;
  const char *aired_from;

  if (anime_obj == NULL || series == NULL) {
    return;
  }

  // Get total episodes
  series->release.total_count =
      (int)ani_json_object_get_int(anime_obj, "episodes", -1);

  // Get latest aired date from aired.from
  aired_obj = ani_json_object_get(anime_obj, "aired");
  if (aired_obj != NULL) {
    from_obj = ani_json_object_get(aired_obj, "from");
    aired_from = ani_json_get_string(from_obj);
    if (aired_from != NULL) {
      ani_parse_iso8601(aired_from, &series->release.latest_date);
    }
  }

  // Get status
  val = ani_json_object_get(anime_obj, "status");
  if (val != NULL) {
    const char *status = ani_json_get_string(val);
    LOG_DEBUG("Anime status: %s", status ? status : "unknown");
  }
}

bool ani_jikan_search_anime(const char *query, ani_series *series) {
  char url[512];
  char *encoded_query;
  ani_http_response *resp;
  ani_json_doc *doc;
  ani_json_val *root;
  ani_json_val *data_array;
  ani_json_val *anime_obj;
  ani_json_val *mal_id_val;
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
  snprintf(url, sizeof(url), JIKAN_SEARCH_URL, encoded_query);
  free(encoded_query);

  LOG_DEBUG("Jikan search: %s", url);

  // Make HTTP request
  resp = ani_http_get(url, NULL);
  if (resp == NULL || resp->status_code != 200) {
    LOG_ERROR("Jikan search failed: HTTP %ld", resp ? resp->status_code : 0);
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
      anime_obj = ani_json_array_get(data_array, 0);
      if (anime_obj != NULL) {
        // Get MAL ID
        mal_id_val = ani_json_object_get(anime_obj, "mal_id");
        if (mal_id_val != NULL) {
          long mal_id = ani_json_get_int(mal_id_val);
          char mal_id_str[32];
          snprintf(mal_id_str, sizeof(mal_id_str), "%ld", mal_id);
          series->id = ani_strdup(mal_id_str);
        }

        // Parse titles and details
        parse_titles(anime_obj, series);
        parse_details(anime_obj, series);

        series->media_type = ANI_MEDIA_ANIME;
        series->provider = ani_strdup("jikan");
        success = true;

        LOG_INFO("Found anime: %s (MAL ID: %s)",
                 series->title.canonical ? series->title.canonical : "unknown",
                 series->id ? series->id : "unknown");
      }
    }
  }

  ani_json_doc_free(doc);
  return success;
}
