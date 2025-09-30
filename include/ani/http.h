/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_HTTP_H
#define ANI_HTTP_H

#include <stdbool.h>
#include <stddef.h>

// HTTP response structure
typedef struct {
  long status_code;
  char *body;
  size_t body_len;
  char *error;
} ani_http_response;

// HTTP client configuration
typedef struct {
  long connect_timeout_ms; // Connect timeout (default: 5000)
  long timeout_ms;         // Overall timeout (default: 15000)
  int max_retries;         // Max retries on 429/5xx (default: 3)
  const char *user_agent;  // User-Agent string
  bool verify_ssl;         // Verify SSL certificates (default: true)
} ani_http_config;

// Initialize HTTP subsystem (call once at startup)
void ani_http_init(void);

// Cleanup HTTP subsystem (call once at exit)
void ani_http_cleanup(void);

// Get default HTTP configuration
ani_http_config ani_http_default_config(void);

// Perform HTTP GET request
ani_http_response *ani_http_get(const char *url, const ani_http_config *config);

// Perform HTTP POST request
ani_http_response *ani_http_post(const char *url, const char *body,
                                 const char *content_type,
                                 const ani_http_config *config);

// Free HTTP response
void ani_http_response_free(ani_http_response *resp);

#endif // ANI_HTTP_H
