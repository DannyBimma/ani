/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/output.h"
#include "ani/time.h"
#include <stdio.h>
#include <string.h>
#include <yyjson.h>

void ani_output_print_series(const ani_series *series) {
  if (series == NULL) {
    return;
  }

  // Print section header
  if (series->media_type == ANI_MEDIA_ANIME) {
    printf("Anime\n");
  } else {
    printf("Manga\n");
  }

  // Print titles
  if (series->title.canonical != NULL) {
    printf("  Title:     %s\n", series->title.canonical);
  }
  if (series->title.english != NULL &&
      !(series->title.canonical &&
        strcmp(series->title.english, series->title.canonical) == 0)) {
    printf("  Title (EN): %s\n", series->title.english);
  }
  if (series->title.japanese != NULL) {
    printf("  Title (JA): %s\n", series->title.japanese);
  }

  // Print total count
  if (series->release.total_count > 0) {
    if (series->media_type == ANI_MEDIA_ANIME) {
      printf("  Episodes:  %d\n", series->release.total_count);
    } else {
      printf("  Chapters:  %d\n", series->release.total_count);
    }
  } else {
    if (series->media_type == ANI_MEDIA_ANIME) {
      printf("  Episodes:  Unknown\n");
    } else {
      printf("  Chapters:  Unknown\n");
    }
  }

  // Print latest release
  if (series->release.latest_number > 0) {
    if (series->media_type == ANI_MEDIA_ANIME) {
      printf("  Latest:    Ep %d", series->release.latest_number);
    } else {
      printf("  Latest:    Ch %d", series->release.latest_number);
    }
    if (series->release.latest_date.year > 0) {
      char date_buf[64];
      ani_format_date(&series->release.latest_date, date_buf, sizeof(date_buf));
      printf(" — %s", date_buf);
    }
    printf("\n");
  }

  // Print next release
  if (series->release.next_number > 0) {
    if (series->media_type == ANI_MEDIA_ANIME) {
      printf("  Next:      Ep %d", series->release.next_number);
    } else {
      printf("  Next:      Ch %d", series->release.next_number);
    }
    if (series->release.next_date.year > 0) {
      char date_buf[64];
      ani_format_date(&series->release.next_date, date_buf, sizeof(date_buf));
      printf(" — %s", date_buf);
    }
    if (series->release.provider_name != NULL) {
      printf(" (source: %s)", series->release.provider_name);
    }
    printf("\n");
  } else {
    printf("  Next:      TBA/Unknown\n");
  }

  printf("\n");
}

void ani_output_print_result(const ani_result *result) {
  if (result == NULL) {
    return;
  }

  if (result->has_anime && result->anime != NULL) {
    ani_output_print_series(result->anime);
  }

  if (result->has_manga && result->manga != NULL) {
    ani_output_print_series(result->manga);
  }

  if (!result->has_anime && !result->has_manga) {
    printf("No results found for \"%s\"\n", result->query ? result->query : "");
  }
}

void ani_output_print_json(const ani_result *result) {
  if (result == NULL) {
    return;
  }

  yyjson_mut_doc *doc = yyjson_mut_doc_new(NULL);
  if (doc == NULL) {
    return;
  }
  yyjson_mut_val *root = yyjson_mut_obj(doc);
  yyjson_mut_doc_set_root(doc, root);

  // Query
  if (result->query) {
    yyjson_mut_obj_add_str(doc, root, "query", result->query);
  } else {
    yyjson_mut_obj_add_null(doc, root, "query");
  }

  // Anime section
  if (result->has_anime && result->anime != NULL) {
    const ani_series *a = result->anime;
    yyjson_mut_val *obj = yyjson_mut_obj(doc);
    if (a->title.english) {
      yyjson_mut_obj_add_str(doc, obj, "title_en", a->title.english);
    } else {
      yyjson_mut_obj_add_null(doc, obj, "title_en");
    }
    if (a->title.japanese) {
      yyjson_mut_obj_add_str(doc, obj, "title_ja", a->title.japanese);
    } else {
      yyjson_mut_obj_add_null(doc, obj, "title_ja");
    }
    if (a->release.total_count > 0) {
      yyjson_mut_obj_add_int(doc, obj, "total_episodes",
                             a->release.total_count);
    } else {
      yyjson_mut_obj_add_null(doc, obj, "total_episodes");
    }

    if (a->release.latest_date.year > 0) {
      char buf[64];
      ani_format_date(&a->release.latest_date, buf, sizeof(buf));
      yyjson_mut_val *latest = yyjson_mut_obj(doc);
      yyjson_mut_obj_add_int(doc, latest, "number", a->release.latest_number);
      yyjson_mut_obj_add_str(doc, latest, "date", buf);
      yyjson_mut_obj_add(obj, yyjson_mut_str(doc, "latest"), latest);
    } else {
      yyjson_mut_obj_add_null(doc, obj, "latest");
    }

    if (a->release.next_date.year > 0) {
      char buf[64];
      ani_format_date(&a->release.next_date, buf, sizeof(buf));
      yyjson_mut_val *next = yyjson_mut_obj(doc);
      yyjson_mut_obj_add_int(doc, next, "number", a->release.next_number);
      yyjson_mut_obj_add_str(doc, next, "date", buf);
      yyjson_mut_obj_add(obj, yyjson_mut_str(doc, "next"), next);
    } else {
      yyjson_mut_obj_add_null(doc, obj, "next");
    }

    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "anime"), obj);
  }

  // Manga section
  if (result->has_manga && result->manga != NULL) {
    const ani_series *m = result->manga;
    yyjson_mut_val *obj = yyjson_mut_obj(doc);
    if (m->title.english) {
      yyjson_mut_obj_add_str(doc, obj, "title_en", m->title.english);
    } else {
      yyjson_mut_obj_add_null(doc, obj, "title_en");
    }
    if (m->title.japanese) {
      yyjson_mut_obj_add_str(doc, obj, "title_ja", m->title.japanese);
    } else {
      yyjson_mut_obj_add_null(doc, obj, "title_ja");
    }
    if (m->release.total_count > 0) {
      yyjson_mut_obj_add_int(doc, obj, "total_chapters",
                             m->release.total_count);
    } else {
      yyjson_mut_obj_add_null(doc, obj, "total_chapters");
    }

    if (m->release.latest_date.year > 0) {
      char buf[64];
      ani_format_date(&m->release.latest_date, buf, sizeof(buf));
      yyjson_mut_val *latest = yyjson_mut_obj(doc);
      yyjson_mut_obj_add_int(doc, latest, "number", m->release.latest_number);
      yyjson_mut_obj_add_str(doc, latest, "date", buf);
      yyjson_mut_obj_add(obj, yyjson_mut_str(doc, "latest"), latest);
    } else {
      yyjson_mut_obj_add_null(doc, obj, "latest");
    }

    if (m->release.next_date.year > 0) {
      char buf[64];
      ani_format_date(&m->release.next_date, buf, sizeof(buf));
      yyjson_mut_val *next = yyjson_mut_obj(doc);
      yyjson_mut_obj_add_int(doc, next, "number", m->release.next_number);
      yyjson_mut_obj_add_str(doc, next, "date", buf);
      yyjson_mut_obj_add(obj, yyjson_mut_str(doc, "next"), next);
    } else {
      yyjson_mut_obj_add_null(doc, obj, "next");
    }

    yyjson_mut_obj_add(root, yyjson_mut_str(doc, "manga"), obj);
  }

  // Write and print
  yyjson_write_err werr;
  char *json =
      yyjson_mut_write_opts(doc, YYJSON_WRITE_PRETTY, NULL, NULL, &werr);
  if (json) {
    printf("%s\n", json);
    free(json);
  }
  yyjson_mut_doc_free(doc);
}
