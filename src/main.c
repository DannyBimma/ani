/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/version.h"
#include "ani/log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

static void
print_version(void)
{
	printf("%s\n", ani_build_info());
}

static void
print_usage(const char *prog)
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

int
main(int argc, char **argv)
{
	/* Set locale for UTF-8 */
	setlocale(LC_ALL, "");

	/* Parse basic flags */
	if (argc < 2) {
		LOG_INFO("No arguments provided. Starting interactive mode...");
		printf("Interactive mode not yet implemented.\n");
		return 0;
	}

	/* Check for --version or -V */
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-V") == 0) {
			print_version();
			return 0;
		}
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			print_usage(argv[0]);
			return 0;
		}
	}

	LOG_INFO("ani starting...");
	LOG_DEBUG("Debug logging enabled");

	/* Placeholder - full implementation coming */
	printf("ani: query processing not yet implemented\n");

	return 0;
}
