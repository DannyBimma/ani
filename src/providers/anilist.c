/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/providers/anilist.h"
#include "ani/http.h"
#include "ani/json.h"
#include "ani/log.h"
#include "ani/str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANILIST_GRAPHQL_URL "https://graphql.anilist.co"

bool ani_anilist_get_next_episode(const char *mal_id, ani_series *series) {
  char query_body[1024];
  ani_http_response *resp;
  ani_json_doc *doc;
  ani_json_val *root;
  ani_json_val *data;
  ani_json_val *media;
  ani_json_val *next_airing;
  ani_json_val *val;
  long airing_at;
  long episode_num;
  bool success;

  if (mal_id == NULL || series == NULL) {
    return false;
  }

  // Build GraphQL query
  snprintf(query_body, sizeof(query_body),
           "{"
           "\"query\": \"query($idMal:Int!){ "
           "Media(idMal:$idMal,type:ANIME){ "
           "id idMal "
           "nextAiringEpisode{ episode airingAt timeUntilAiring } "
           "title{ romaji english native } "
           "} }\","
           "\"variables\": {\"idMal\": %s}"
           "}",
           mal_id);

  LOG_DEBUG("AniList GraphQL query for MAL ID: %s", mal_id);

  // Make HTTP POST request
  resp =
      ani_http_post(ANILIST_GRAPHQL_URL, query_body, "application/json", NULL);
  if (resp == NULL || resp->status_code != 200) {
    LOG_WARN("AniList query failed: HTTP %ld", resp ? resp->status_code : 0);
    ani_http_response_free(resp);
    return false;
  }

  // Parse JSON response
  doc = ani_json_parse(resp->body, resp->body_len);
  ani_http_response_free(resp);

  if (doc == NULL) {
    return false;
  }

  success = false;
  root = ani_json_get_root(doc);
  if (root != NULL) {
    data = ani_json_object_get(root, "data");
    if (data != NULL) {
      media = ani_json_object_get(data, "Media");
      if (media != NULL) {
        next_airing = ani_json_object_get(media, "nextAiringEpisode");
        if (next_airing != NULL) {
          // Get episode number
          val = ani_json_object_get(next_airing, "episode");
          if (val != NULL) {
            episode_num = ani_json_get_int(val);
            series->release.next_number = (int)episode_num;
          }

          // Get airing timestamp (Unix seconds)
          val = ani_json_object_get(next_airing, "airingAt");
          if (val != NULL) {
            airing_at = ani_json_get_int(val);
            ani_parse_unix_timestamp(airing_at, &series->release.next_date);
          }

          // Set source metadata
          series->release.next_source = ANI_SOURCE_AGGREGATED_API;
          series->release.next_confidence = ANI_CONFIDENCE_OFFICIAL;
          free(series->release.provider_name);
          series->release.provider_name = ani_strdup("AniList");

          success = true;
          LOG_INFO("Found next episode: Ep %d via AniList",
                   series->release.next_number);
        } else {
          LOG_DEBUG("No upcoming episode found for MAL ID %s", mal_id);
        }
      }
    }
  }

  ani_json_doc_free(doc);
  return success;
}
