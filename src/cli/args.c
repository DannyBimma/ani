/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/cli.h"
#include "ani/version.h"
#include "ani/str.h"
#include "ani/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
ani_cli_print_version(void)
{
	printf("%s\n", ani_build_info());
}

void
ani_cli_print_usage(const char *prog)
{
	printf("Usage: %s [options] <query...>\n\n", prog);
	printf("Options:\n");
	printf("  -m, --manga          Query manga only\n");
	printf("  -a, --anime          Query anime only\n");
	printf("  -b, --both           Query both (default)\n");
	printf("  -j, --json           Output JSON in addition to human format\n");
	printf("  -r, --refresh        Bypass cache\n");
	printf("  -t, --timeout <ms>   HTTP timeout override\n");
	printf("  -v, --verbose        Verbose logs (repeat for debug: -vv)\n");
	printf("  --official-only      Only official schedule sources\n");
	printf("  --scrape-ok          Allow HTML parsing for official sites\n");
	printf("  -V, --version        Print version and build info\n");
	printf("  -h, --help           Show this help\n\n");
	printf("Examples:\n");
	printf("  %s One Piece\n", prog);
	printf("  %s \"Demon Slayer\" -a\n", prog);
	printf("  %s Berserk -m --json\n", prog);
}

bool
ani_cli_parse_args(int argc, char **argv, ani_cli_options *opts)
{
	int i;
	int query_start;
	int query_count;
	const char **query_parts;

	if (opts == NULL) {
		return false;
	}

	/* Initialize defaults */
	memset(opts, 0, sizeof(*opts));
	opts->query_both = true;
	opts->timeout_ms = -1;

	/* Parse flags */
	query_start = -1;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--manga") == 0) {
			opts->query_manga = true;
			opts->query_both = false;
		} else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--anime") == 0) {
			opts->query_anime = true;
			opts->query_both = false;
		} else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--both") == 0) {
			opts->query_both = true;
			opts->query_anime = false;
			opts->query_manga = false;
		} else if (strcmp(argv[i], "-j") == 0 || strcmp(argv[i], "--json") == 0) {
			opts->output_json = true;
		} else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--refresh") == 0) {
			opts->refresh_cache = true;
		} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
			opts->verbose_level++;
		} else if (strcmp(argv[i], "--official-only") == 0) {
			opts->official_only = true;
		} else if (strcmp(argv[i], "--scrape-ok") == 0) {
			opts->scrape_ok = true;
		} else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--timeout") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "Error: --timeout requires an argument\n");
				return false;
			}
			opts->timeout_ms = atol(argv[++i]);
		} else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			return false; /* Let caller handle help */
		} else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
			return false; /* Let caller handle version */
		} else if (argv[i][0] == '-') {
			fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
			return false;
		} else {
			/* Start of query */
			query_start = i;
			break;
		}
	}

	/* Join query parts */
	if (query_start >= 0) {
		query_count = argc - query_start;
		query_parts = malloc(sizeof(char *) * (size_t)query_count);
		if (query_parts == NULL) {
			return false;
		}

		for (i = 0; i < query_count; i++) {
			query_parts[i] = argv[query_start + i];
		}

		opts->query = ani_str_join(query_parts, (size_t)query_count, " ");
		free(query_parts);

		if (opts->query == NULL) {
			return false;
		}
	}

	return true;
}

void
ani_cli_options_free(ani_cli_options *opts)
{
	if (opts == NULL) {
		return;
	}

	free(opts->query);
	opts->query = NULL;
}
