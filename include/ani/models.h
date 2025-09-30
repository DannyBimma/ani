/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_MODELS_H
#define ANI_MODELS_H

#include "ani/time.h"
#include <stdbool.h>

/* Media type */
typedef enum {
	ANI_MEDIA_ANIME,
	ANI_MEDIA_MANGA
} ani_media_type;

/* Schedule source */
typedef enum {
	ANI_SOURCE_OFFICIAL_PLATFORM,
	ANI_SOURCE_AGGREGATED_API,
	ANI_SOURCE_STREAMING_PLATFORM,
	ANI_SOURCE_MAGAZINE_CALENDAR,
	ANI_SOURCE_CADENCE_HEURISTIC,
	ANI_SOURCE_BROADCAST,
	ANI_SOURCE_UNKNOWN
} ani_schedule_source;

/* Confidence level */
typedef enum {
	ANI_CONFIDENCE_OFFICIAL,
	ANI_CONFIDENCE_ESTIMATED,
	ANI_CONFIDENCE_LOW
} ani_confidence;

/* Title information */
typedef struct {
	char *english;
	char *japanese;
	char *canonical;
} ani_title;

/* Release information */
typedef struct {
	ani_date latest_date;
	ani_date next_date;
	int latest_number;      /* Episode/chapter number, -1 if unknown */
	int next_number;        /* Next episode/chapter number, -1 if unknown */
	int total_count;        /* Total episodes/chapters, -1 if unknown */
	ani_schedule_source next_source;
	ani_confidence next_confidence;
	char *provider_name;    /* e.g., "ANILIST", "MANGAPLUS" */
} ani_release_info;

/* Series information */
typedef struct {
	char *id;               /* Provider-specific ID */
	ani_title title;
	ani_media_type media_type;
	ani_release_info release;
	char *provider;         /* Provider name (e.g., "jikan", "mangadex") */
} ani_series;

/* Query result (holds both anime and manga) */
typedef struct {
	char *query;
	ani_series *anime;
	ani_series *manga;
	bool has_anime;
	bool has_manga;
} ani_result;

/* Allocate and free functions */
ani_series *ani_series_new(void);
void ani_series_free(ani_series *series);

ani_result *ani_result_new(void);
void ani_result_free(ani_result *result);

/* Helper to set title */
void ani_title_set(ani_title *title, const char *english,
                   const char *japanese, const char *canonical);

/* Helper to free title strings */
void ani_title_free(ani_title *title);

#endif /* ANI_MODELS_H */
