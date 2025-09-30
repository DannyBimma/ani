/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_OUTPUT_H
#define ANI_OUTPUT_H

#include "ani/models.h"
#include <stdbool.h>

// Print series information in human-readable format
void ani_output_print_series(const ani_series *series);

// Print result (anime and/or manga) in human-readable format
void ani_output_print_result(const ani_result *result);

// Print result in JSON format
void ani_output_print_json(const ani_result *result);

#endif // ANI_OUTPUT_H
