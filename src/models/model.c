/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/models.h"
#include "ani/str.h"
#include <stdlib.h>
#include <string.h>

ani_series *
ani_series_new(void)
{
	ani_series *series;

	series = calloc(1, sizeof(*series));
	if (series == NULL) {
		return NULL;
	}

	series->release.latest_number = -1;
	series->release.next_number = -1;
	series->release.total_count = -1;
	series->release.next_source = ANI_SOURCE_UNKNOWN;
	series->release.next_confidence = ANI_CONFIDENCE_LOW;

	return series;
}

void
ani_title_free(ani_title *title)
{
	if (title == NULL) {
		return;
	}

	free(title->english);
	free(title->japanese);
	free(title->canonical);

	title->english = NULL;
	title->japanese = NULL;
	title->canonical = NULL;
}

void
ani_series_free(ani_series *series)
{
	if (series == NULL) {
		return;
	}

	free(series->id);
	ani_title_free(&series->title);
	free(series->release.provider_name);
	free(series->provider);
	free(series);
}

ani_result *
ani_result_new(void)
{
	ani_result *result;

	result = calloc(1, sizeof(*result));
	return result;
}

void
ani_result_free(ani_result *result)
{
	if (result == NULL) {
		return;
	}

	free(result->query);
	ani_series_free(result->anime);
	ani_series_free(result->manga);
	free(result);
}

void
ani_title_set(ani_title *title, const char *english,
              const char *japanese, const char *canonical)
{
	if (title == NULL) {
		return;
	}

	/* Free existing */
	ani_title_free(title);

	/* Set new */
	if (english != NULL) {
		title->english = ani_strdup(english);
	}
	if (japanese != NULL) {
		title->japanese = ani_strdup(japanese);
	}
	if (canonical != NULL) {
		title->canonical = ani_strdup(canonical);
	}
}
