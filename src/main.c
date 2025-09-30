/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/version.h"
#include "ani/log.h"
#include "ani/cli.h"
#include "ani/output.h"
#include "ani/http.h"
#include "ani/models.h"
#include "ani/providers/jikan.h"
#include "ani/providers/anilist.h"
#include "ani/providers/mangadex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

static int
process_query(const ani_cli_options *opts)
{
	ani_result *result;
	bool anime_success;
	bool manga_success;

	if (opts == NULL || opts->query == NULL) {
		fprintf(stderr, "Error: No query provided\n");
		return 1;
	}

	result = ani_result_new();
	if (result == NULL) {
		fprintf(stderr, "Error: Failed to allocate result\n");
		return 1;
	}

	result->query = strdup(opts->query);

	/* Query anime if requested */
	if (opts->query_both || opts->query_anime) {
		LOG_INFO("Searching for anime: %s", opts->query);

		result->anime = ani_series_new();
		if (result->anime != NULL) {
			anime_success = ani_jikan_search_anime(opts->query, result->anime);
			if (anime_success) {
				result->has_anime = true;

				/* Get next episode schedule from AniList */
				if (result->anime->id != NULL) {
					ani_anilist_get_next_episode(result->anime->id, result->anime);
				}
			} else {
				LOG_WARN("Anime search failed or no results");
				ani_series_free(result->anime);
				result->anime = NULL;
			}
		}
	}

	/* Query manga if requested */
	if (opts->query_both || opts->query_manga) {
		LOG_INFO("Searching for manga: %s", opts->query);

		result->manga = ani_series_new();
		if (result->manga != NULL) {
			manga_success = ani_mangadex_search_manga(opts->query, result->manga);
			if (manga_success) {
				result->has_manga = true;

				/* Get latest chapter info */
				if (result->manga->id != NULL) {
					ani_mangadex_get_latest_chapter(result->manga->id, result->manga);
				}
			} else {
				LOG_WARN("Manga search failed or no results");
				ani_series_free(result->manga);
				result->manga = NULL;
			}
		}
	}

	/* Output results */
	if (opts->output_json) {
		ani_output_print_json(result);
	} else {
		ani_output_print_result(result);
	}

	/* Cleanup */
	ani_result_free(result);

	return 0;
}

int
main(int argc, char **argv)
{
	ani_cli_options opts;
	int ret;

	/* Set locale for UTF-8 */
	setlocale(LC_ALL, "");

	/* Initialize HTTP subsystem */
	ani_http_init();

	/* Handle no arguments - interactive mode */
	if (argc < 2) {
		printf("Interactive mode not yet implemented.\n");
		printf("Try: %s --help\n", argv[0]);
		ani_http_cleanup();
		return 0;
	}

	/* Parse arguments */
	if (!ani_cli_parse_args(argc, argv, &opts)) {
		/* Check for version/help flags */
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-V") == 0) {
				ani_cli_print_version();
				ani_http_cleanup();
				return 0;
			}
			if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
				ani_cli_print_usage(argv[0]);
				ani_http_cleanup();
				return 0;
			}
		}

		/* Parse error */
		ani_cli_print_usage(argv[0]);
		ani_http_cleanup();
		return 1;
	}

	/* Set log level based on verbosity */
	if (opts.verbose_level == 0) {
		ani_log_set_level(ANI_LOG_WARN);
	} else if (opts.verbose_level == 1) {
		ani_log_set_level(ANI_LOG_INFO);
	} else {
		ani_log_set_level(ANI_LOG_DEBUG);
	}

	/* Require query */
	if (opts.query == NULL) {
		fprintf(stderr, "Error: No query provided\n");
		ani_cli_print_usage(argv[0]);
		ani_cli_options_free(&opts);
		ani_http_cleanup();
		return 1;
	}

	/* Process query */
	ret = process_query(&opts);

	/* Cleanup */
	ani_cli_options_free(&opts);
	ani_http_cleanup();

	return ret;
}
