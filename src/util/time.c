/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/time.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

bool
ani_parse_iso8601(const char *str, ani_date *out)
{
	int n;
	char tz_sign;
	int tz_hours;
	int tz_minutes;

	if (str == NULL || out == NULL) {
		return false;
	}

	memset(out, 0, sizeof(*out));
	out->hour = -1;
	out->minute = -1;
	out->second = -1;
	out->offset_minutes = 0;
	out->has_time = false;

	/* Try YYYY-MM-DD format first */
	n = sscanf(str, "%d-%d-%d", &out->year, &out->month, &out->day);
	if (n == 3) {
		/* Validate date */
		if (out->year < 1900 || out->year > 2100 ||
		    out->month < 1 || out->month > 12 ||
		    out->day < 1 || out->day > 31) {
			return false;
		}

		/* Check for time component */
		const char *t_pos = strchr(str, 'T');
		if (t_pos == NULL) {
			return true;
		}

		t_pos++; /* Skip 'T' */

		/* Parse time: hh:mm:ss */
		n = sscanf(t_pos, "%d:%d:%d", &out->hour, &out->minute, &out->second);
		if (n >= 2) {
			out->has_time = true;
			if (n == 2) {
				out->second = 0;
			}

			/* Validate time */
			if (out->hour < 0 || out->hour > 23 ||
			    out->minute < 0 || out->minute > 59 ||
			    out->second < 0 || out->second > 59) {
				return false;
			}

			/* Check for timezone */
			const char *tz_pos = strpbrk(t_pos, "Zz+-");
			if (tz_pos != NULL) {
				if (*tz_pos == 'Z' || *tz_pos == 'z') {
					out->offset_minutes = 0;
				} else {
					tz_sign = *tz_pos;
					n = sscanf(tz_pos + 1, "%d:%d", &tz_hours, &tz_minutes);
					if (n >= 1) {
						out->offset_minutes = tz_hours * 60;
						if (n == 2) {
							out->offset_minutes += tz_minutes;
						}
						if (tz_sign == '-') {
							out->offset_minutes = -out->offset_minutes;
						}
					}
				}
			}
		}

		return true;
	}

	return false;
}

bool
ani_parse_unix_timestamp(long timestamp, ani_date *out)
{
	time_t t;
	struct tm *tm_info;

	if (out == NULL) {
		return false;
	}

	t = (time_t)timestamp;
	tm_info = gmtime(&t);
	if (tm_info == NULL) {
		return false;
	}

	out->year = tm_info->tm_year + 1900;
	out->month = tm_info->tm_mon + 1;
	out->day = tm_info->tm_mday;
	out->hour = tm_info->tm_hour;
	out->minute = tm_info->tm_min;
	out->second = tm_info->tm_sec;
	out->offset_minutes = 0; /* UTC */
	out->has_time = true;

	return true;
}

void
ani_format_date(const ani_date *date, char *buf, size_t size)
{
	if (date == NULL || buf == NULL || size == 0) {
		return;
	}

	snprintf(buf, size, "%04d-%02d-%02d", date->year, date->month, date->day);
}

void
ani_format_datetime(const ani_date *date, char *buf, size_t size)
{
	char offset_str[16];

	if (date == NULL || buf == NULL || size == 0) {
		return;
	}

	if (!date->has_time || date->hour < 0) {
		ani_format_date(date, buf, size);
		return;
	}

	if (date->offset_minutes == 0) {
		snprintf(offset_str, sizeof(offset_str), "Z");
	} else {
		int abs_offset = date->offset_minutes < 0 ? -date->offset_minutes : date->offset_minutes;
		int hours = abs_offset / 60;
		int minutes = abs_offset % 60;
		snprintf(offset_str, sizeof(offset_str), "%c%02d:%02d",
		         date->offset_minutes < 0 ? '-' : '+', hours, minutes);
	}

	snprintf(buf, size, "%04d-%02d-%02dT%02d:%02d:%02d%s",
	         date->year, date->month, date->day,
	         date->hour, date->minute, date->second,
	         offset_str);
}
