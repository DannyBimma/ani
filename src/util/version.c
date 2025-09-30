/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */

#include "ani/version.h"

const char *
ani_version(void)
{
	return ANI_VERSION;
}

const char *
ani_build_info(void)
{
	return "ani " ANI_VERSION " (C99, yyjson, libcurl)"
#ifdef ANI_WITH_UTF8PROC
		" +utf8proc"
#endif
#ifdef __SANITIZE_ADDRESS__
		" +ASAN"
#endif
	;
}
