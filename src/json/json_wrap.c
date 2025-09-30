/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/json.h"
#include "ani/log.h"
#include <stdlib.h>
#include <string.h>
#include <yyjson.h>

// Internal document structure
struct ani_json_doc {
  yyjson_doc *doc;
};

// Internal value structure (just wraps yyjson_val)
struct ani_json_val {
  yyjson_val *val;
};

ani_json_doc *ani_json_parse(const char *json_str, size_t len) {
  ani_json_doc *doc;
  yyjson_read_err err;

  if (json_str == NULL || len == 0) {
    return NULL;
  }

  doc = malloc(sizeof(*doc));
  if (doc == NULL) {
    return NULL;
  }

  doc->doc = yyjson_read_opts((char *)json_str, len, 0, NULL, &err);
  if (doc->doc == NULL) {
    LOG_ERROR("JSON parse error: %s (at position %zu)", err.msg, err.pos);
    free(doc);
    return NULL;
  }

  return doc;
}

void ani_json_doc_free(ani_json_doc *doc) {
  if (doc == NULL) {
    return;
  }

  yyjson_doc_free(doc->doc);
  free(doc);
}

ani_json_val *ani_json_get_root(ani_json_doc *doc) {
  static struct ani_json_val root_wrapper;

  if (doc == NULL || doc->doc == NULL) {
    return NULL;
  }

  root_wrapper.val = yyjson_doc_get_root(doc->doc);
  if (root_wrapper.val == NULL) {
    return NULL;
  }

  return &root_wrapper;
}

bool ani_json_is_object(const ani_json_val *val) {
  return val != NULL && val->val != NULL && yyjson_is_obj(val->val);
}

bool ani_json_is_array(const ani_json_val *val) {
  return val != NULL && val->val != NULL && yyjson_is_arr(val->val);
}

bool ani_json_is_string(const ani_json_val *val) {
  return val != NULL && val->val != NULL && yyjson_is_str(val->val);
}

bool ani_json_is_int(const ani_json_val *val) {
  return val != NULL && val->val != NULL &&
         (yyjson_is_int(val->val) || yyjson_is_uint(val->val));
}

bool ani_json_is_bool(const ani_json_val *val) {
  return val != NULL && val->val != NULL && yyjson_is_bool(val->val);
}

bool ani_json_is_null(const ani_json_val *val) {
  return val == NULL || val->val == NULL || yyjson_is_null(val->val);
}

ani_json_val *ani_json_object_get(const ani_json_val *obj, const char *key) {
  static struct ani_json_val result;
  yyjson_val *val;

  if (!ani_json_is_object(obj) || key == NULL) {
    return NULL;
  }

  val = yyjson_obj_get(obj->val, key);
  if (val == NULL) {
    return NULL;
  }

  result.val = val;
  return &result;
}

const char *ani_json_object_get_string(const ani_json_val *obj,
                                       const char *key) {
  ani_json_val *val;

  val = ani_json_object_get(obj, key);
  return ani_json_get_string(val);
}

long ani_json_object_get_int(const ani_json_val *obj, const char *key,
                             long default_val) {
  ani_json_val *val;

  val = ani_json_object_get(obj, key);
  if (val == NULL || !ani_json_is_int(val)) {
    return default_val;
  }

  return ani_json_get_int(val);
}

bool ani_json_object_get_bool(const ani_json_val *obj, const char *key,
                              bool default_val) {
  ani_json_val *val;

  val = ani_json_object_get(obj, key);
  if (val == NULL || !ani_json_is_bool(val)) {
    return default_val;
  }

  return ani_json_get_bool(val);
}

size_t ani_json_array_size(const ani_json_val *arr) {
  if (!ani_json_is_array(arr)) {
    return 0;
  }

  return yyjson_arr_size(arr->val);
}

ani_json_val *ani_json_array_get(const ani_json_val *arr, size_t index) {
  static struct ani_json_val result;
  yyjson_val *val;

  if (!ani_json_is_array(arr)) {
    return NULL;
  }

  val = yyjson_arr_get(arr->val, index);
  if (val == NULL) {
    return NULL;
  }

  result.val = val;
  return &result;
}

const char *ani_json_get_string(const ani_json_val *val) {
  if (!ani_json_is_string(val)) {
    return NULL;
  }

  return yyjson_get_str(val->val);
}

long ani_json_get_int(const ani_json_val *val) {
  if (!ani_json_is_int(val)) {
    return 0;
  }

  if (yyjson_is_int(val->val)) {
    return (long)yyjson_get_sint(val->val);
  } else {
    return (long)yyjson_get_uint(val->val);
  }
}

bool ani_json_get_bool(const ani_json_val *val) {
  if (!ani_json_is_bool(val)) {
    return false;
  }

  return yyjson_get_bool(val->val);
}

const char *ani_json_get_string_safe(const ani_json_val *val,
                                     const char *default_val) {
  const char *str;

  str = ani_json_get_string(val);
  return str != NULL ? str : default_val;
}
