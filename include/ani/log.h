/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_LOG_H
#define ANI_LOG_H

#include <stdio.h>

/* Log levels */
typedef enum {
	ANI_LOG_ERROR = 0,
	ANI_LOG_WARN  = 1,
	ANI_LOG_INFO  = 2,
	ANI_LOG_DEBUG = 3
} ani_log_level;

/* Global log level */
extern ani_log_level ani_log_current_level;

/* Set log level (0=error, 1=warn, 2=info, 3=debug) */
void ani_log_set_level(ani_log_level level);

/* Log functions */
void ani_log_error(const char *fmt, ...);
void ani_log_warn(const char *fmt, ...);
void ani_log_info(const char *fmt, ...);
void ani_log_debug(const char *fmt, ...);

/* Convenience macros */
#define LOG_ERROR(...) ani_log_error(__VA_ARGS__)
#define LOG_WARN(...)  ani_log_warn(__VA_ARGS__)
#define LOG_INFO(...)  ani_log_info(__VA_ARGS__)
#define LOG_DEBUG(...) ani_log_debug(__VA_ARGS__)

#endif /* ANI_LOG_H */
