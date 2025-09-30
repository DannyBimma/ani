/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_CLI_H
#define ANI_CLI_H

#include <stdbool.h>

// CLI options
typedef struct {
  bool query_anime;
  bool query_manga;
  bool query_both;
  bool output_json;
  bool refresh_cache;
  bool official_only;
  bool scrape_ok;
  int verbose_level; // 0=default, 1=info, 2=debug
  long timeout_ms;
  char *query; // Joined query string
} ani_cli_options;

// Parse command-line arguments
bool ani_cli_parse_args(int argc, char **argv, ani_cli_options *opts);

// Free CLI options
void ani_cli_options_free(ani_cli_options *opts);

// Print usage
void ani_cli_print_usage(const char *prog);

// Print version
void ani_cli_print_version(void);

#endif // ANI_CLI_H
