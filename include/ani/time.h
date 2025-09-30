/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_TIME_H
#define ANI_TIME_H

#include <time.h>
#include <stdbool.h>

/* Parsed date/time structure */
typedef struct {
	int year;
	int month;   /* 1-12 */
	int day;     /* 1-31 */
	int hour;    /* 0-23, -1 if not present */
	int minute;  /* 0-59, -1 if not present */
	int second;  /* 0-59, -1 if not present */
	int offset_minutes; /* Timezone offset in minutes from UTC, 0 if UTC/unknown */
	bool has_time;
} ani_date;

/* Parse ISO-8601 date: YYYY-MM-DD or YYYY-MM-DDThh:mm:ss[Z|±hh:mm] */
bool ani_parse_iso8601(const char *str, ani_date *out);

/* Parse Unix timestamp (seconds since epoch) */
bool ani_parse_unix_timestamp(long timestamp, ani_date *out);

/* Format date as ISO-8601 string (YYYY-MM-DD) */
void ani_format_date(const ani_date *date, char *buf, size_t size);

/* Format date with time as ISO-8601 string */
void ani_format_datetime(const ani_date *date, char *buf, size_t size);

#endif /* ANI_TIME_H */
