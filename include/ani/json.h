/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#ifndef ANI_JSON_H
#define ANI_JSON_H

#include <stdbool.h>
#include <stddef.h>

// Opaque JSON document and value types
typedef struct ani_json_doc ani_json_doc;
typedef struct ani_json_val ani_json_val;

// Parse JSON string into document
ani_json_doc *ani_json_parse(const char *json_str, size_t len);

// Free JSON document
void ani_json_doc_free(ani_json_doc *doc);

// Get root value from document
ani_json_val *ani_json_get_root(ani_json_doc *doc);

// Type checking
bool ani_json_is_object(const ani_json_val *val);
bool ani_json_is_array(const ani_json_val *val);
bool ani_json_is_string(const ani_json_val *val);
bool ani_json_is_int(const ani_json_val *val);
bool ani_json_is_bool(const ani_json_val *val);
bool ani_json_is_null(const ani_json_val *val);

// Object accessors
ani_json_val *ani_json_object_get(const ani_json_val *obj, const char *key);
const char *ani_json_object_get_string(const ani_json_val *obj,
                                       const char *key);
long ani_json_object_get_int(const ani_json_val *obj, const char *key,
                             long default_val);
bool ani_json_object_get_bool(const ani_json_val *obj, const char *key,
                              bool default_val);

// Array accessors
size_t ani_json_array_size(const ani_json_val *arr);
ani_json_val *ani_json_array_get(const ani_json_val *arr, size_t index);

// Value getters
const char *ani_json_get_string(const ani_json_val *val);
long ani_json_get_int(const ani_json_val *val);
bool ani_json_get_bool(const ani_json_val *val);

// Safe string accessor with default
const char *ani_json_get_string_safe(const ani_json_val *val,
                                     const char *default_val);

#endif // ANI_JSON_H
