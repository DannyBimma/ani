/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/log.h"
#include <stdarg.h>
#include <time.h>

ani_log_level ani_log_current_level = ANI_LOG_INFO;

void ani_log_set_level(ani_log_level level) { ani_log_current_level = level; }

static void ani_log_internal(ani_log_level level, const char *level_str,
                             const char *fmt, va_list args) {
  if (level > ani_log_current_level) {
    return;
  }

  time_t now;
  struct tm *tm_info;
  char time_buf[32];

  time(&now);
  tm_info = localtime(&now);
  strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

  fprintf(stderr, "[%s] %s: ", time_buf, level_str);
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
}

void ani_log_error(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  ani_log_internal(ANI_LOG_ERROR, "ERROR", fmt, args);
  va_end(args);
}

void ani_log_warn(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  ani_log_internal(ANI_LOG_WARN, "WARN", fmt, args);
  va_end(args);
}

void ani_log_info(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  ani_log_internal(ANI_LOG_INFO, "INFO", fmt, args);
  va_end(args);
}

void ani_log_debug(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  ani_log_internal(ANI_LOG_DEBUG, "DEBUG", fmt, args);
  va_end(args);
}
