/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/http.h"
#include "ani/log.h"
#include "ani/str.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Buffer for accumulating response data
typedef struct {
  char *data;
  size_t size;
  size_t capacity;
} ani_http_buffer;

static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp) {
  size_t realsize;
  ani_http_buffer *buf;
  char *new_data;

  realsize = size * nmemb;
  buf = (ani_http_buffer *)userp;

  // Grow buffer if needed
  if (buf->size + realsize + 1 > buf->capacity) {
    size_t new_capacity = buf->capacity * 2;
    if (new_capacity < buf->size + realsize + 1) {
      new_capacity = buf->size + realsize + 1;
    }

    new_data = realloc(buf->data, new_capacity);
    if (new_data == NULL) {
      LOG_ERROR("Failed to allocate memory for HTTP response");
      return 0;
    }

    buf->data = new_data;
    buf->capacity = new_capacity;
  }

  memcpy(buf->data + buf->size, contents, realsize);
  buf->size += realsize;
  buf->data[buf->size] = '\0';

  return realsize;
}

void ani_http_init(void) { curl_global_init(CURL_GLOBAL_DEFAULT); }

void ani_http_cleanup(void) { curl_global_cleanup(); }

ani_http_config ani_http_default_config(void) {
  ani_http_config config;

  config.connect_timeout_ms = 5000;
  config.timeout_ms = 15000;
  config.max_retries = 3;
  config.user_agent = "ani/0.1.0 (https://github.com/DannyBimma/ani)";
  config.verify_ssl = true;

  return config;
}

static ani_http_response *
ani_http_request_internal(const char *url,
                          const char *method __attribute__((unused)),
                          const char *post_body, const char *content_type,
                          const ani_http_config *config) {
  CURL *curl;
  CURLcode res;
  ani_http_buffer buf;
  ani_http_response *resp;
  int retry;
  struct curl_slist *headers;
  long retry_after;

  if (url == NULL) {
    return NULL;
  }

  // Use default config if not provided
  if (config == NULL) {
    static ani_http_config default_config;
    default_config = ani_http_default_config();
    config = &default_config;
  }

  // Initialize response buffer
  buf.capacity = 4096;
  buf.data = malloc(buf.capacity);
  buf.size = 0;
  if (buf.data == NULL) {
    return NULL;
  }
  buf.data[0] = '\0';

  // Create response structure
  resp = calloc(1, sizeof(*resp));
  if (resp == NULL) {
    free(buf.data);
    return NULL;
  }

  curl = curl_easy_init();
  if (curl == NULL) {
    free(buf.data);
    free(resp);
    return NULL;
  }

  // Set URL
  curl_easy_setopt(curl, CURLOPT_URL, url);

  // Set timeouts
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, config->connect_timeout_ms);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, config->timeout_ms);

  // Set User-Agent
  if (config->user_agent != NULL) {
    curl_easy_setopt(curl, CURLOPT_USERAGENT, config->user_agent);
  }

  // SSL verification
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, config->verify_ssl ? 1L : 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, config->verify_ssl ? 2L : 0L);

  // Enable compression
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

  // Follow redirects
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

  // Set write callback
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

  // Set method and body for POST
  headers = NULL;
  if (post_body != NULL) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(post_body));

    if (content_type != NULL) {
      char header[256];
      snprintf(header, sizeof(header), "Content-Type: %s", content_type);
      headers = curl_slist_append(headers, header);
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
  }

  // Retry loop
  for (retry = 0; retry <= config->max_retries; retry++) {
    if (retry > 0) {
      LOG_DEBUG("Retrying request (attempt %d/%d): %s", retry + 1,
                config->max_retries + 1, url);
      // Exponential backoff (cross-platform)
#ifdef _WIN32
      Sleep((DWORD)(1000U * (1U << (retry - 1))));
#else
      sleep((unsigned int)(1U << (retry - 1)));
#endif
    }

    // Reset buffer for retry
    buf.size = 0;
    buf.data[0] = '\0';

    // Perform request
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      LOG_ERROR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
      resp->error = ani_strdup(curl_easy_strerror(res));
      continue; // Retry
    }

    // Get status code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp->status_code);

    LOG_DEBUG("HTTP %ld %s", resp->status_code, url);

    // Check for rate limit (429) or server error (5xx)
    if (resp->status_code == 429 || resp->status_code >= 500) {
      // Check for Retry-After header
      retry_after = 0;
      curl_easy_getinfo(curl, CURLINFO_RETRY_AFTER, &retry_after);
      if (retry_after > 0 && retry_after < 60) {
        LOG_WARN("Rate limited, waiting %ld seconds", retry_after);
        // Cross-platform sleep
#ifdef _WIN32
        Sleep((DWORD)(retry_after * 1000));
#else
        sleep((unsigned int)retry_after);
#endif
      }

      if (retry < config->max_retries) {
        LOG_WARN("HTTP %ld, retrying", resp->status_code);
        continue;
      }
    }

    // Success
    break;
  }

  // Cleanup
  if (headers != NULL) {
    curl_slist_free_all(headers);
  }
  curl_easy_cleanup(curl);

  // Set response body
  resp->body = buf.data;
  resp->body_len = buf.size;

  return resp;
}

ani_http_response *ani_http_get(const char *url,
                                const ani_http_config *config) {
  return ani_http_request_internal(url, "GET", NULL, NULL, config);
}

ani_http_response *ani_http_post(const char *url, const char *body,
                                 const char *content_type,
                                 const ani_http_config *config) {
  return ani_http_request_internal(url, "POST", body, content_type, config);
}

void ani_http_response_free(ani_http_response *resp) {
  if (resp == NULL) {
    return;
  }

  free(resp->body);
  free(resp->error);
  free(resp);
}
