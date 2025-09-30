/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/output.h"
#include "ani/time.h"
#include <stdio.h>
#include <string.h>

void
ani_output_print_series(const ani_series *series)
{
	if (series == NULL) {
		return;
	}

	/* Print section header */
	if (series->media_type == ANI_MEDIA_ANIME) {
		printf("Anime\n");
	} else {
		printf("Manga\n");
	}

	/* Print titles */
	if (series->title.canonical != NULL) {
		printf("  Title:     %s\n", series->title.canonical);
	}
	if (series->title.english != NULL && series->title.english != series->title.canonical) {
		printf("  Title (EN): %s\n", series->title.english);
	}
	if (series->title.japanese != NULL) {
		printf("  Title (JA): %s\n", series->title.japanese);
	}

	/* Print total count */
	if (series->release.total_count > 0) {
		if (series->media_type == ANI_MEDIA_ANIME) {
			printf("  Episodes:  %d\n", series->release.total_count);
		} else {
			printf("  Chapters:  %d\n", series->release.total_count);
		}
	} else {
		if (series->media_type == ANI_MEDIA_ANIME) {
			printf("  Episodes:  Unknown\n");
		} else {
			printf("  Chapters:  Unknown\n");
		}
	}

	/* Print latest release */
	if (series->release.latest_number > 0) {
		if (series->media_type == ANI_MEDIA_ANIME) {
			printf("  Latest:    Ep %d", series->release.latest_number);
		} else {
			printf("  Latest:    Ch %d", series->release.latest_number);
		}
		if (series->release.latest_date.year > 0) {
			char date_buf[64];
			ani_format_date(&series->release.latest_date, date_buf, sizeof(date_buf));
			printf(" — %s", date_buf);
		}
		printf("\n");
	}

	/* Print next release */
	if (series->release.next_number > 0) {
		if (series->media_type == ANI_MEDIA_ANIME) {
			printf("  Next:      Ep %d", series->release.next_number);
		} else {
			printf("  Next:      Ch %d", series->release.next_number);
		}
		if (series->release.next_date.year > 0) {
			char date_buf[64];
			ani_format_date(&series->release.next_date, date_buf, sizeof(date_buf));
			printf(" — %s", date_buf);
		}
		if (series->release.provider_name != NULL) {
			printf(" (source: %s)", series->release.provider_name);
		}
		printf("\n");
	} else {
		printf("  Next:      TBA/Unknown\n");
	}

	printf("\n");
}

void
ani_output_print_result(const ani_result *result)
{
	if (result == NULL) {
		return;
	}

	if (result->has_anime && result->anime != NULL) {
		ani_output_print_series(result->anime);
	}

	if (result->has_manga && result->manga != NULL) {
		ani_output_print_series(result->manga);
	}

	if (!result->has_anime && !result->has_manga) {
		printf("No results found for \"%s\"\n", result->query ? result->query : "");
	}
}

void
ani_output_print_json(const ani_result *result)
{
	char date_buf[64];

	if (result == NULL) {
		return;
	}

	printf("{\n");
	printf("  \"query\": \"%s\",\n", result->query ? result->query : "");

	/* Anime section */
	if (result->has_anime && result->anime != NULL) {
		ani_series *anime = result->anime;

		printf("  \"anime\": {\n");
		printf("    \"title_en\": %s,\n",
		       anime->title.english ? anime->title.english : "null");
		printf("    \"title_ja\": %s,\n",
		       anime->title.japanese ? anime->title.japanese : "null");
		printf("    \"total_episodes\": %d,\n",
		       anime->release.total_count > 0 ? anime->release.total_count : 0);

		if (anime->release.latest_date.year > 0) {
			ani_format_date(&anime->release.latest_date, date_buf, sizeof(date_buf));
			printf("    \"latest\": { \"number\": %d, \"date\": \"%s\" },\n",
			       anime->release.latest_number, date_buf);
		} else {
			printf("    \"latest\": null,\n");
		}

		if (anime->release.next_date.year > 0) {
			ani_format_date(&anime->release.next_date, date_buf, sizeof(date_buf));
			printf("    \"next\": { \"number\": %d, \"date\": \"%s\" }\n",
			       anime->release.next_number, date_buf);
		} else {
			printf("    \"next\": null\n");
		}

		printf("  }");
		if (result->has_manga) {
			printf(",");
		}
		printf("\n");
	}

	/* Manga section */
	if (result->has_manga && result->manga != NULL) {
		ani_series *manga = result->manga;

		printf("  \"manga\": {\n");
		printf("    \"title_en\": %s,\n",
		       manga->title.english ? manga->title.english : "null");
		printf("    \"title_ja\": %s,\n",
		       manga->title.japanese ? manga->title.japanese : "null");
		printf("    \"total_chapters\": %d,\n",
		       manga->release.total_count > 0 ? manga->release.total_count : 0);

		if (manga->release.latest_date.year > 0) {
			ani_format_date(&manga->release.latest_date, date_buf, sizeof(date_buf));
			printf("    \"latest\": { \"number\": %d, \"date\": \"%s\" },\n",
			       manga->release.latest_number, date_buf);
		} else {
			printf("    \"latest\": null,\n");
		}

		if (manga->release.next_date.year > 0) {
			ani_format_date(&manga->release.next_date, date_buf, sizeof(date_buf));
			printf("    \"next\": { \"number\": %d, \"date\": \"%s\" }\n",
			       manga->release.next_number, date_buf);
		} else {
			printf("    \"next\": null\n");
		}

		printf("  }\n");
	}

	printf("}\n");
}
