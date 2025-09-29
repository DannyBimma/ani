# ani — Production-Ready Build Plan (ANSI C CLI)

> Anime and manga tracking CLI focused on fast, safe, minimal-dependency execution with robust UX and predictable API integrations.

## 1) Objectives

- Deliver a single binary CLI `ani` in ANSI C (C99), portable across Linux, macOS, and Windows (MSYS2/MinGW), with no runtime GC and minimal dependencies.
- Provide fast, memory-safe HTTP + JSON processing, strict error handling, and pleasant UX.
- Comply with K&R style for functions/blocks; C99 `//` single-line comments; traditional `/* ... */` for multi-line.
- Include traditional copyright/program header at the top of all source files.

### File Header Template

```c
/*
 * Routine: ani — Anime/Manga scheduling information CLI
 * Author: DannyBimma
 * Copyright: (c) 2025 Technomancer Pirate Captain. All Rights Reserved.
 */
```

## 2) CLI Behaviour

- Command: `ani [options] <query...>`
- Query: collected from all non-flag args and joined with spaces (handles unquoted multi-word input like `One Piece`).
- Flags:
  - `-m, --manga` — query manga only.
  - `-a, --anime` — query anime only.
  - `-b, --both` — default; query both if available.
  - `-j, --json` — output raw JSON summary (machine-readable) in addition to human format.
  - `-r, --refresh` — bypass cache and hit APIs.
  - `-t, --timeout <ms>` — HTTP timeout override.
  - `-v, --verbose` — verbose logs; repeat (`-vv`) for debug.
  - `--official-only` — only official schedule sources (no magazine/heuristic fallbacks).
  - `--scrape-ok` — allow HTML parsing for official sites (e.g., VIZ) when no JSON endpoint is available.
  - `-V, --version` — print version and build info.
  - `-h, --help` — usage help.
  - Notes: `--official-only` still uses trusted aggregated APIs for anime (AniList AiringSchedule), but disables magazine/heuristic fallbacks.
- No args: interactive prompt using a safe line reader (`fgets` loop with dynamic growth) asking for series name.
- Case-insensitive processing: ASCII-insensitive matching and Unicode normalization (best-effort); Japanese input handled as UTF‑8.
- Friendly errors for missing data/endpoints and network failures.

### Output Requirements Mapping

For `-m` (manga) or `-a` (anime), output:

- Official title in Japanese and English (best available from API)
- Total chapters/episodes (if provided by API; else “Unknown”)
- Release date of current latest chapter/episode
- Release date of next scheduled chapter/episode (or “Unknown/TBA” when the API lacks it)

If no flag provided: fetch and display both anime and manga (concurrently) when available.

## 3) Architecture Overview

- `cli`: argument parsing, prompt mode, output rendering (human + JSON).
- `core`: orchestrates providers, caching, normalization, and merging results.
- `providers`: pluggable fetch/parse modules per API (Anime: Jikan + AniList for schedule; Manga: MangaDex primary; optional FireFlyDeveloper Manga-API adapter if stable).
- `net`: HTTP client wrapper around libcurl, with timeouts, TLS, retries, rate-limiting backoff.
- `json`: thin wrapper around chosen JSON library with safety helpers.
- `models`: strongly-typed C structs for Series, Episode/Chapter, Title variants, Dates.
- `util`: UTF‑8 normalization/case-fold, safe string ops, logging, time/date parsing, platform paths.

Concurrency: when querying both anime and manga, use `libcurl` multi interface to issue requests concurrently.

## 4) Dependencies (lean, vetted)

- HTTP: `libcurl` (HTTPS, HTTP/2, robust, widely available). TLS via system OpenSSL/SecureTransport/Schannel.
- JSON: Preferred `yyjson` (fast, C, DOM + pointer safety). Fallback `jsmn` (minimal, token-based) selectable via build option:
  - Default: `-DANI_JSON=yyjson` (vendored) for speed and reliability.
  - Fallback: `-DANI_JSON=jsmn` to minimize footprint.
- Unicode normalization/case-fold: `utf8proc` vendored (optional). Without it, fall back to UTF‑8 pass-through with ASCII-insensitive match only.
- Testing: `Unity` test framework vendored (tiny, C-only) + CTest.

All third-party code vendored under `third_party/` with license files preserved.

## 5) Data Model (C structs)

- `ani_media_type`: `ANIME` | `MANGA`.
- `ani_title`: English, Japanese, canonical (pointers to immutable strings).
- `ani_date`: ISO-8601 parse result (YYYY-MM-DD), supports unknown fields; timezone stored as offset minutes.
- `ani_release_info`: latest_date, next_date, latest_number (episode/chapter), totals (nullable), next_source (enum), next_confidence (enum).
- `ani_series`: id (provider ID), titles (array of `ani_title`), media_type, release_info, provider tag.
- `ani_result`: holds both anime and manga results for a query; includes source JSON snippet when `--json`.

Memory: explicit ownership annotations; string storage in arena per operation; single exit cleanup pattern.

Enumerations:

- `ani_schedule_source`: `OFFICIAL_PLATFORM`, `AGGREGATED_API`, `STREAMING_PLATFORM`, `MAGAZINE_CALENDAR`, `CADENCE_HEURISTIC`, `BROADCAST` (anime), `UNKNOWN`.
- `ani_confidence`: `OFFICIAL`, `ESTIMATED`, `LOW`.

## 6) Providers

### 6.1 Anime — Jikan (MyAnimeList)

- Search: `GET https://api.jikan.moe/v4/anime?q=<q>&limit=1&order_by=popularity` (configurable `limit` fallback to first best match).
- Detail: `GET /v4/anime/{id}` → titles, total episodes (`episodes`), aired/broadcast info.
- Episodes: `GET /v4/anime/{id}/episodes?limit=100&order_by=aired` (iterate pages if needed) → for latest aired date and count; some titles have future schedules; otherwise infer next via broadcast cadence when possible.
- Rate limit/backoff per Jikan docs (respect `Retry-After`).

Limitations: Jikan may not expose authoritative next episode schedules. See 6.6 for the dedicated next‑episode provider chain using AniList and fallbacks. When all sources fail, show “TBA/Unknown”.

### 6.2 Manga — MangaDex (Primary)

- Search: `GET https://api.mangadex.org/manga?title=<q>&limit=1&order=relevance` → pick best match by localized titles.
- Latest chapter: `GET /chapter?manga=<id>&translatedLanguage[]=en&limit=1&order[publishAt]=desc` → extract chapter number + publish date.
- Totals: Use `GET /manga/<id>/aggregate` to compute known chapter count; if absent, show “Unknown”.
- Next scheduled: resolved via a provider chain (see 6.5) prioritizing official platforms (e.g., MangaPlus) and publisher calendars; falls back to cadence-based estimation with explicit confidence labeling.

### 6.3 Optional — FireFlyDeveloper/Manga-API adapter

- If API proves stable and documented, add adapter behind provider interface, controlled by `--provider manga-api` or config.

### 6.4 Provider Interface (C)

```c
typedef struct ani_provider_vtbl {
    const char *name;
    int  (*search_best)(const char *query, ani_series *out, ani_log *log);
    void (*destroy)(ani_series *s);
} ani_provider_vtbl;
```

### 6.5 Manga Next‑Chapter Scheduling — Provider Chain

- Goal: provide upcoming chapter dates where possible with provenance and confidence.
- Order of resolution (short-circuit on first authoritative hit):
  1. MangaPlus (Shueisha) official schedule
     - Detection: use MangaDex `links.engtl` or `links` hints (e.g., `engtl` pointing to MangaPlus/VIZ) or Jikan `/v4/manga` external links to locate MangaPlus `title_id`.
     - Fetch: call MangaPlus public JSON endpoints used by the web app to retrieve chapter list and upcoming releases (fields include scheduled publish timestamps). Map to next chapter number + UTC date.
     - Label: `next_source=OFFICIAL_PLATFORM`, `next_confidence=OFFICIAL`.
  2. VIZ Shonen Jump schedule (English simulpub)
     - Detection: `engtl` link domain matches `viz.com/shonenjump`.
     - Fetch: parse JSON data embedded on the title’s page (JSON-LD/microdata) that includes upcoming chapter availability windows.
     - Compliance: enable only when `--scrape-ok` or config allows HTML parsing; otherwise skipped.
     - Label: `next_source=OFFICIAL_PLATFORM`, `next_confidence=OFFICIAL`.
  3. Publisher calendars by magazine
     - Lookup: query Jikan `/v4/manga` to obtain `serializations` (e.g., “Weekly Shōnen Jump”, “Young Magazine”).
     - Map: use a curated `magazine_calendars.json` (shipped/updated with releases) containing authoritative upcoming issue dates for major magazines (6–12 months horizon).
     - Compute: next chapter date = next issue date; annotate as estimate since series may skip issues.
     - Label: `next_source=MAGAZINE_CALENDAR`, `next_confidence=ESTIMATED`.
  4. Cadence-based statistical estimate
     - Data: take last N official chapter publish dates from MangaPlus/VIZ when available, else MangaDex chapter `publishAt`.
     - Method: compute median interval (days); next date = last_date + median_interval; cap confidence low.
     - Label: `next_source=CADENCE_HEURISTIC`, `next_confidence=LOW`.

- Fallback behavior: if all sources fail, return “TBA/Unknown”.
- Testing: record fixtures for MangaPlus JSON, VIZ page JSON, and magazine calendar snapshots; assert mapping and labeling.
- TOS/ethics: avoid aggressive scraping; default to official JSON endpoints; require opt-in for HTML parsing.

### 6.6 Anime Next‑Episode Scheduling — Provider Chain

- Goal: provide upcoming episode dates/airtimes with provenance and confidence.
- Order of resolution (short-circuit on first authoritative hit):
  1. AniList GraphQL AiringSchedule (preferred)
     - Mapping: use MAL ID from Jikan (`data.mal_id`) to query AniList via `idMal` field.
     - Endpoint: `POST https://graphql.anilist.co` with JSON body:
       - Query: `query($idMal:Int!){ Media(idMal:$idMal,type:ANIME){ id idMal nextAiringEpisode{ episode airingAt timeUntilAiring } title{ romaji english native } } }`
       - Extract: `nextAiringEpisode.episode` and `airingAt` (Unix seconds, UTC) → map to next episode number and ISO-8601 date.
     - Label: `next_source=AGGREGATED_API` with provider name `ANILIST`, `next_confidence=OFFICIAL` (trusted aggregate of official times).
     - Rate limits: respect 429 with backoff; cache 15–30 min TTL.
  2. Streaming platform schedules (optional)
     - Crunchyroll (region-dependent): if a title mapping exists (via external links on Jikan/AniList), optionally parse schedule JSON from public endpoints; enable only when `--scrape-ok`.
     - Label: `next_source=STREAMING_PLATFORM`, `next_confidence=OFFICIAL`.
  3. Schedule aggregators via HTML (optional)
     - LiveChart/AnimeSchedule pages often embed JSON-LD with airing times; only used when `--scrape-ok`.
     - Disambiguation: require season+year+title match to reduce false positives.
     - Label: `next_source=AGGREGATED_API`, `next_confidence=ESTIMATED`.
  4. Broadcast cadence heuristic
     - Data: Jikan `broadcast` field (weekday+time+timezone) and recent aired episode date(s).
     - Compute: next date by advancing to the next scheduled weekday/time after the latest aired timestamp; adjust for known skips (holidays) only when magazine calendar indicates a break for adaptations.
     - Label: `next_source=BROADCAST`, `next_confidence=ESTIMATED`.

- Fallback: if all fail, report “TBA/Unknown”.
- Testing: fixtures for AniList GraphQL responses and broadcast-cadence edge cases.
- Compliance: prefer official/GraphQL API; HTML parsing disabled by default and guarded by `--scrape-ok`.

## 7) JSON Parsing

- Default: `yyjson` DOM parsing, immediate extraction to typed structs, then free the DOM to minimize live memory.
- Fallback: `jsmn` token parsing with zero-copy references into a single read buffer; convert only required fields; careful bounds checks.
- Strict validation: type checks, range checks, known keys only; ignore unknown keys gracefully.

## 8) HTTP & Networking

- `libcurl` easy + multi API, connection reuse enabled, compressed responses accepted.
- Timeouts: connect 5s, overall 15s default; configurable.
- Retries: exponential backoff with jitter on 429/5xx, max 3 attempts; obey `Retry-After`.
- TLS: verify peer and host; no insecure fallbacks; CA from system.
- User-Agent: `ani/<version> (+https://github.com/... )` and distinct per provider if requested.

## 9) Caching

- Location: XDG (`$XDG_CACHE_HOME/ani`) or `~/.cache/ani` (Linux), `~/Library/Caches/ani` (macOS), `%LOCALAPPDATA%\ani\Cache` (Windows).
- Key: provider + normalized query hash (SHA-256, hex) + versioned schema suffix.
- TTL: search results 5 min; details 6 h; schedule lookups 15–30 min for AniList; 30–60 min for other sources (shorter for official platform schedules). `--refresh` bypasses cache.
- Format: JSON files storing raw provider response + extracted summary for resilience.
- Offline: if network fails, use cache with clear “offline cache” notice.

## 10) Input, Unicode, Normalization

- Assume UTF‑8 everywhere; call `setlocale(LC_ALL, "")` at startup.
- Reader: safe line input function using `fgets` in a loop to grow buffer until newline.
- Normalization: If `utf8proc` present, apply NFKC + case-fold for searching; retain original for display. Without it, perform ASCII-only case-insensitive compare and pass UTF‑8 as-is to APIs.

## 11) Output Formatting

- Human: aligned labels, ISO-8601 dates w/ timezone abbreviation when available, unknown fields printed as “Unknown” or “TBA”. Show provenance when available, e.g., “(source: MangaPlus official)” or “(estimated from Weekly Shōnen Jump calendar)”.
- JSON: minimal stable schema:

```json
{
  "query": "One Piece",
  "anime": {
    "title_en": "One Piece",
    "title_ja": "ワンピース",
    "total_episodes": 1100,
    "latest": { "number": 1100, "date": "2025-01-12" },
    "next": {
      "number": 1101,
      "date": "2025-01-19",
      "source": "ANILIST",
      "confidence": "OFFICIAL"
    }
  },
  "manga": {
    "title_en": "One Piece",
    "title_ja": "ONE PIECE",
    "total_chapters": 1110,
    "latest": { "number": 1110, "date": "2025-01-10" },
    "next": {
      "number": 1111,
      "date": "2025-01-17",
      "source": "MANGAPLUS",
      "confidence": "OFFICIAL"
    }
  }
}
```

- Errors: friendly, actionable messages with suggestions (e.g., retry, refine query, `--refresh`).

## 12) Errors, Safety, and Style

- K&R style; strict warnings: `-std=c99 -Wall -Wextra -Werror -Wshadow -Wconversion -pedantic`.
- Prefer `snprintf`, length-checked copies; provide `ani_strlcpy` portable shim if needed.
- Single-exit cleanup with `goto` error label; every allocation has a clear owner and `free` path.
- Logging macros with levels; DEBUG logs can include HTTP status/body snippets (redact on error if needed).
- Defensive parsing: bounds checking, integer overflow checks, time parsing validation.

## 13) Build & Tooling

- Build system: CMake (3.16+). Targets: `ani`, `ani_tests`.
- Options: `-DANI_JSON=yyjson|jsmn`, `-DANI_WITH_UTF8PROC=ON|OFF`, `-DANI_SANITIZE=ON`.
- Sanitizers (dev): Address/UB (`-fsanitize=address,undefined`), with `-O1 -g3`.
- Static analysis: `cppcheck`, optional `clang-tidy` config.
- Formatting: `.clang-format` tuned for K&R brace placement; enforced via CI (advisory, not blocking release unless egregious).

## 14) Testing & QA

- Unit tests: util (strings, dates), JSON mapping, provider mappers.
- Integration tests: recorded HTTP fixtures (JSON files) to avoid live API during CI.
- Airing schedule fixtures: AniList GraphQL responses (nextAiringEpisode) and edge cases (no schedule, special episodes).
- CLI tests: golden output comparisons for representative queries and error paths.
- Memory checks: valgrind on Linux; `leaks` on macOS CI job; sanitizer builds.
- Rate limit tests: simulate 429 with Retry-After.

## 15) Security & Privacy

- TLS verification on by default; no insecure option.
- Respect API policies; backoff on rate limits.
- No telemetry; minimal logs; redact sensitive headers if any.
- Vendored deps with checksums; pin versions.

## 16) Directory Layout

```
ani/
  CMakeLists.txt
  src/
    main.c
    cli/args.c, prompt.c, output.c
    core/ani.c, cache.c, normalize.c
    providers/
      jikan.c
      anilist.c
      mangadex.c
      manga_api_firefly.c (optional)
    net/http.c
    json/json_wrap.c
    models/types.h, models/model.c
    util/{log.c, str.c, fs.c, time.c}
  include/ani/*.h  (public headers)
  third_party/{yyjson/, jsmn/, utf8proc/, unity/}
  tests/{unit/, integration/}
  fixtures/{jikan/, mangadex/}
  scripts/{dev/, release/}
  docs/
```

## 17) Implementation Phases

1. Bootstrap project

- CMake skeleton, third_party wiring, `libcurl` detection, `yyjson` default.
- `ani --version` and basic logging.

2. Core utilities

- Safe string/IO helpers, time parser, platform paths, logging macros.

3. HTTP layer

- libcurl easy + multi wrappers, timeouts, retries, gzip, UA.

4. JSON layer

- `yyjson` wrapper; fallback `jsmn` path builds.

5. Provider: Jikan

- Search, details, episodes aggregation, mapping to models.

6. Provider: MangaDex

- Search, latest chapter, aggregate totals.

7. CLI and prompt

- Args parsing, interactive prompt, output formatting, `--json`.

  7.5. Manga next‑chapter scheduling

- Implement provider chain (MangaPlus → VIZ → magazine calendars → cadence heuristic) with provenance/confidence tagging.

  7.6. Anime next‑episode scheduling

- Implement provider chain (AniList GraphQL → streaming platform schedules → schedule aggregators [opt-in] → broadcast cadence heuristic) with provenance/confidence tagging.

8. Caching

- Keying, TTLs, offline fallback, refresh.

9. Unicode normalization

- Integrate `utf8proc` and fallbacks; normalization pipeline.

10. Tests & hardening

- Unit/integration tests, fixtures, mem/sanitizer runs, CI.

11. Packaging & release

- Static builds where reasonable; Homebrew tap formula; versioning/tagging.

## 18) Mapping “Usage” to Implementation

- `./ani One Piece -m` → parse flags, join `One` + `Piece`, hit Manga provider only, print titles (EN/JA), totals, latest/next dates.
- `./ani One Piece -a` → Anime provider only.
- `./ani` → prompt with safe reader, then default `--both` using concurrent requests.
- No flag: run both providers concurrently and print both sections if available.

## 19) Date Handling

- Parse ISO-8601 (`YYYY-MM-DD[Thh:mm[:ss][Z|±hh:mm]]`), store UTC + offset.
- Output local date by default, with ISO UTC when `--json`.
- Handle missing/partial dates gracefully.

## 20) Error Messages (examples)

- Network: “Network error: TLS handshake failed. Try `--refresh` or check connection.”
- Not found: “No matching series found for ‘<q>’. Try a different title or add `--manga`/`--anime`.”
- Schedule unknown: “Next release date is not provided by the source (TBA).”

## 21) Risks & Mitigations

- Limited coverage for upcoming chapters/episodes → Use prioritized chains (AniList/official platforms for anime; MangaPlus/VIZ/mag calendars for manga) with clear provenance and confidence, and allow opt-out of non-official parsing.
- API or site changes → versioned cache schema, provider isolation, recorded fixtures to detect breakage quickly; feature flags to disable affected providers.
- TOS/compliance concerns for HTML parsing → Disabled by default; require `--scrape-ok` or config; prefer official JSON endpoints.
- GraphQL rate limits (AniList) → exponential backoff with jitter; cache aggressively (15–30 min); parallel-query batcher to coalesce identical lookups.
- Unicode complexity → optional `utf8proc` path; pass-through UTF‑8 otherwise.
- Windows path/console quirks → normalize paths, UTF‑8 code page opt-in docs.

## 22) CI Pipeline (GitHub Actions suggestion)

- Matrix: linux/mac/windows, Debug+ASAN and Release.
- Steps: configure → build → unit/integration (fixtures) → valgrind/leaks (OS-specific) → artifact upload.

## 23) Performance Notes

- Avoid large DOM lifetimes; extract-then-free.
- Reuse `CURL` handles; share DNS cache; enable HTTP/2 where available.
- Allocate arenas per request to simplify frees and reduce fragmentation.

## 24) Coding Conventions Summary

- K&R braces, 100-col width, snake_case identifiers, `const` correctness.
- Header guards, no global mutable state; pass contexts explicitly.
- Single-exit cleanup pattern; never ignore return values.

## 25) Example Output (Human)

```
$ ani One Piece

Anime
- Title (EN): One Piece
- Title (JA): ワンピース
- Episodes: 1100
- Latest: Ep 1100 — 2025-01-12
- Next:   Ep 1101 — 2025-01-19 (source: AniList schedule)

Manga
- Title (EN): One Piece
- Title (JA): ONE PIECE
- Chapters: 1110
- Latest: Ch 1110 — 2025-01-10
- Next:   Ch 1111 — 2025-01-17 (source: MangaPlus official)
```

## 26) Next Steps

- Initialize CMake skeleton and scaffolding per layout.
- Vendor `yyjson`, `utf8proc`, `Unity`; wire provider interfaces.
- Implement Jikan + MangaDex minimal happy path and fixtures.
- Add caching and CLI polishing.
- Harden with tests, sanitizers, and CI.

---

This plan emphasizes safe C practices, predictable behavior across platforms, and API/provider isolation to keep the codebase maintainable and resilient.
