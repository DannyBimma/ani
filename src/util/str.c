/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/str.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t ani_strlcpy(char *dst, const char *src, size_t size) {
  size_t src_len;

  if (size == 0) {
    return strlen(src);
  }

  src_len = strlen(src);
  if (src_len < size) {
    memcpy(dst, src, src_len + 1);
  } else {
    memcpy(dst, src, size - 1);
    dst[size - 1] = '\0';
  }

  return src_len;
}

size_t ani_strlcat(char *dst, const char *src, size_t size) {
  size_t dst_len;
  size_t src_len;

  dst_len = strlen(dst);
  src_len = strlen(src);

  if (dst_len >= size) {
    return size + src_len;
  }

  return dst_len + ani_strlcpy(dst + dst_len, src, size - dst_len);
}

char *ani_str_trim(char *str) {
  char *end;

  // Trim leading space
  while (isspace((unsigned char)*str)) {
    str++;
  }

  if (*str == '\0') {
    return str;
  }

  // Trim trailing space
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) {
    end--;
  }

  end[1] = '\0';

  return str;
}

int ani_strcasecmp(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    int c1 = tolower((unsigned char)*s1);
    int c2 = tolower((unsigned char)*s2);
    if (c1 != c2) {
      return c1 - c2;
    }
    s1++;
    s2++;
  }
  return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

int ani_strncasecmp(const char *s1, const char *s2, size_t n) {
  size_t i;

  for (i = 0; i < n; i++) {
    int c1 = tolower((unsigned char)s1[i]);
    int c2 = tolower((unsigned char)s2[i]);
    if (c1 != c2) {
      return c1 - c2;
    }
    if (s1[i] == '\0') {
      break;
    }
  }
  return 0;
}

char *ani_strdup(const char *s) {
  size_t len;
  char *copy;

  if (s == NULL) {
    return NULL;
  }

  len = strlen(s);
  copy = malloc(len + 1);
  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, s, len + 1);
  return copy;
}

char *ani_str_join(const char **parts, size_t count, const char *sep) {
  size_t total_len;
  size_t sep_len;
  size_t i;
  char *result;
  char *p;

  if (count == 0) {
    return ani_strdup("");
  }

  sep_len = sep ? strlen(sep) : 0;
  total_len = 0;

  for (i = 0; i < count; i++) {
    total_len += strlen(parts[i]);
    if (i < count - 1) {
      total_len += sep_len;
    }
  }

  result = malloc(total_len + 1);
  if (result == NULL) {
    return NULL;
  }

  p = result;
  for (i = 0; i < count; i++) {
    size_t part_len = strlen(parts[i]);
    memcpy(p, parts[i], part_len);
    p += part_len;

    if (i < count - 1 && sep) {
      memcpy(p, sep, sep_len);
      p += sep_len;
    }
  }

  *p = '\0';
  return result;
}

char *ani_readline(void) {
  size_t size;
  size_t len;
  char *buf;
  int c;

  size = 128;
  buf = malloc(size);
  if (buf == NULL) {
    return NULL;
  }

  len = 0;
  while ((c = fgetc(stdin)) != EOF && c != '\n') {
    if (len + 1 >= size) {
      size *= 2;
      char *new_buf = realloc(buf, size);
      if (new_buf == NULL) {
        free(buf);
        return NULL;
      }
      buf = new_buf;
    }
    buf[len++] = (char)c;
  }

  buf[len] = '\0';
  return buf;
}
