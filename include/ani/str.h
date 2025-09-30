/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_STR_H
#define ANI_STR_H

#include <stddef.h>
#include <stdbool.h>

/* Safe string copy (always null-terminates) */
size_t ani_strlcpy(char *dst, const char *src, size_t size);

/* Safe string concatenation */
size_t ani_strlcat(char *dst, const char *src, size_t size);

/* Trim whitespace from start and end */
char *ani_str_trim(char *str);

/* Case-insensitive ASCII comparison */
int ani_strcasecmp(const char *s1, const char *s2);
int ani_strncasecmp(const char *s1, const char *s2, size_t n);

/* Safe string duplication */
char *ani_strdup(const char *s);

/* Join strings with separator */
char *ani_str_join(const char **parts, size_t count, const char *sep);

/* Safe line reader (grows buffer as needed) */
char *ani_readline(void);

#endif /* ANI_STR_H */
