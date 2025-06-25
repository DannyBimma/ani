# Build & Architecture Plan for **ani**

_A command-line assistant for today's seasonal anime schedules_

---

## 1. High-Level Goals

1. Fetch the current season's schedule from the Jikan REST API.
2. Present a concise, readable table of shows for:
   • **today** (no args)  
   • **a given weekday** (arg only)  
   • **a specific show from any past or present season** (arg + `-a` flag)
3. Keep the binary tiny, fast, portable (POSIX / macOS / Linux), with minimal dependencies.
4. Stick to the ANSI-C standard, use helper prototypes to keep main function at the top of the main file, create separate files when necessary to keep things clean and well organised.

---

## 2. Key Decisions

| Concern         | Choice                                         | Rationale                                                               |
| --------------- | ---------------------------------------------- | ----------------------------------------------------------------------- |
| Language        | C90                                            | Reliable C features, wide compiler support.                             |
| HTTP client     | `libcurl`                                      | Mature, portable, easy to build statically.                             |
| JSON parsing    | [`cJSON`](https://github.com/DaveGamble/cJSON) | Small footprint, BSD-licensed.                                          |
| Arg parsing     | `getopt_long()`                                | Standard on glibc & macOS; fallback wrapper for Windows if ever ported. |
| Table rendering | Custom ANSI table helper                       | No extra dependency; supports widths & UTF-8 truncation.                |
| Tests           | `cmocka`                                       | Lightweight unit-test framework for C.                                  |
| Build system    | `CMake`                                        | Cross-platform, easy GitHub Actions integration.                        |
| Caching         | In-memory only (JSON → structs)                | Data is fetched once per run; external cache unnecessary.               |

---

## 3. Functional Flow

```mermaid
flowchart TD
    Start([ani])
    Args{{parse args}}
    Fetch[call Jikan API<br/>/seasons/now | /schedules]
    Map[convert JSON -> structs]

    subgraph CLI modes
        NoArg(No args) --> FilterToday
        DayArg([weekday]) --> FilterDay
        AnimeArg([title, -a]) --> FilterAnime
    end

    FilterToday[filter by today]
    FilterDay[filter by weekday]
    FilterAnime[find show; keep all eps]

    Format[format table]
    Print[stdout]
    Start --> Args --> Fetch --> Map --> CLI modes --> Format --> Print
```

---

## 4. Module-Level Design

| Module          | Header       | Core Functions                                       | Notes                                |
| --------------- | ------------ | ---------------------------------------------------- | ------------------------------------ |
| **main**        | `ani.h`      | `int main(int,char**)`                               | wires everything                     |
| **cli**         | `cli.h`      | `parse_args()`, `usage()`                            | wraps `getopt_long`                  |
| **net**         | `net.h`      | `http_get(url, &char* buf, size_t* len)`             | libcurl init, cleanup                |
| **jikan**       | `jikan.h`    | `fetch_schedule(struct season_s* out)`               | builds URL using current year/season |
| **model**       | `model.h`    | `struct Anime`, `struct Episode`, etc.               | pure data layer                      |
| **filter**      | `filter.h`   | `shows_today()`, `shows_by_day()`, `show_by_title()` | case-insensitive search              |
| **table**       | `table.h`    | `print_table(const struct Anime*, size_t n)`         | dynamic column widths                |
| **util/time**   | `timeutil.h` | `today_weekday()`, `current_season()`                | uses `<time.h>`                      |
| **util/string** | `strutil.h`  | `strcasecmp_utf8()` etc.                             | locale-aware comparison              |
| **tests/**      | —            | unit tests per module                                | CMake `add_test`                     |

All public headers go in `include/`.

---

## 5. Data Structures (simplified)

```c
typedef struct {
    int number;          // episode number
    char airdate[20];    // ISO-8601
    char airtime[10];    // HH:MM
} Episode;

typedef struct {
    char  title[512];
    char  studio[64];
    char  weekday[10];   // "monday"…
    Episode current;     // latest ep
} Anime;

typedef struct {
    char season[8];      // "Winter"
    int  year;
    Anime* list;         // dynamic array
    size_t count;
} SeasonSchedule;
```

---

## 6. Error-Handling Strategy

1. Each module returns explicit status codes (`ANI_OK`, `ANI_ERR_NET`, `ANI_ERR_JSON`, …).
2. `main` maps these to human-readable messages.
3. Graceful degradation: if studio info missing, display "—".

---

## 7. Algorithmic Notes

• **Season detection**

```
month = now.tm_mon+1
season = { Winter:1-3, Spring:4-6, Summer:7-9, Fall:10-12 }
```

• **Weekday mapping**  
Map API weekday strings to lowercase English; internal enum for quick comparisons.

• **Case-insensitive matching**  
Use `strcasecmp` after UTF-8 NFC normalization (simple ASCII for MVP).

• **Table rendering**

1. Scan rows to compute max widths.
2. Print header `Season xxxx (Winter)` centered.
3. Use Unicode box-drawing chars if terminal supports UTF-8; fallback to ASCII.

---

## 8. CLI Syntax & Examples

```text
$ ani
# prints today's shows for current season

$ ani friday
# prints Friday schedule for current season

$ ani "FriDaY"
# case-insensitive

$ ani -a "Sousou no Frieren"
# detailed schedule for that show, and the season it aired in.
# case-insensitive
```

Exit codes: `0` success, `2` bad args, `420` net error, `69` parse error.

---

## 9. Build & CI

1. **CMakeLists.txt**  
   • Detect `libcurl` and `cJSON`.  
   • `option(ANI_BUILD_TESTS "Build unit tests" ON)`
2. **Build**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j
   ```
3. **Static binary (optional)**  
   `-DCMAKE_EXE_LINKER_FLAGS="-static"`
4. **GitHub Actions**  
   • matrix: `ubuntu-latest`, `macos-latest`  
   • steps: cache deps, configure, build, run tests.

---

## 10. Testing Matrix

| Test                | Category    | Description                                |
| ------------------- | ----------- | ------------------------------------------ |
| `net_fetch_ok`      | integration | Mock Jikan with `ncat`; ensure 200 parsed. |
| `parse_schedule`    | unit        | JSON → structs size/count correctness.     |
| `filter_today`      | unit        | Today vs. sample weekday mapping.          |
| `casefold_title`    | unit        | "Frieren" vs "frieren" matches.            |
| `cli_invalid_day`   | CLI         | Return code `2` on gibberish arg.          |
| `print_table_width` | unit        | Long titles wrap/truncate nicely.          |

---

## 11. Future Extensions

1. Persistent cache (`~/.cache/ani.json`) with 24 h TTL.
2. Colorised output with seasons' accent colors.
3. Watch mode (`ani --watch`) to refresh every n minutes.
4. Windows port (WinHTTP, pdcurses for table).
5. Shell completions (bash/zsh/fish).

---

## 12. Project Layout

```
anime-tracker/
├── CMakeLists.txt
├── include/
│   ├── ani.h
│   ├── cli.h         …
│   └── util/
├── src/
│   ├── main.c
│   ├── cli.c
│   ├── net.c         …
├── tests/
│   └── test_cli.c    …
├── third_party/
│   ├── cJSON/
│   └── cmocka/
└── README.md
```

---

### Happy hacking while watching, & enjoy the season's offerings!
