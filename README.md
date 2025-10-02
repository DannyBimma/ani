# ani

ani is a small, fast C99 CLI that shows upcoming anime episodes and latest manga chapters. It queries popular public APIs and prints a concise human view or a JSON representation suitable for scripting.

Features

- Query anime via Jikan (MyAnimeList) with next episode from AniList
- Query manga via MangaDex with latest English chapter info
- Pretty human-readable output or JSON (`-j`) for automation
- Robust HTTP client with retries, gzip, redirect follow, and timeouts
- Cross‑platform (macOS, Linux, Windows) with CMake build

Quick Start

- Build: `make` (Release) or `make debug`
- Run: `make run ARGS='"One Piece" -a'`
- JSON: `make run-json ARGS='Berserk -m'`
- Install to `bin/`: `make install` then run `./bin/ani "Demon Slayer"`

Usage

```
ani [options] <query...>

Options:
  -m, --manga          Query manga only
  -a, --anime          Query anime only
  -b, --both           Query both (default)
  -j, --json           Output JSON (instead of human format)
  -r, --refresh        Bypass cache
  -t, --timeout <ms>   HTTP timeout override
  -v, --verbose        Verbose logs (repeat for debug: -vv)
  --official-only      Only official schedule sources
  --scrape-ok          Allow HTML parsing for official sites
  -V, --version        Print version and build info
  -h, --help           Show this help
```

Examples

- Anime next airing: `./build/src/ani -a "Demon Slayer"`
- Manga latest chapter: `./build/src/ani -m Berserk`
- JSON for scripting: `./build/src/ani -j "One Piece"`

Build Instructions

Requirements

- C compiler with C99 support (Clang or GCC; MSVC works via CMake)
- CMake ≥ 3.16
- libcurl (headers + library)

macOS (Homebrew)

- `brew install cmake curl`
- `make` then run `./build/src/ani "One Piece"`

Ubuntu/Debian

- `sudo apt-get install -y build-essential cmake libcurl4-openssl-dev`
- `make` then run `./build/src/ani Naruto`

Fedora

- `sudo dnf install -y gcc gcc-c++ cmake libcurl-devel`
- `make` then run `./build/src/ani Naruto`

Windows

- Option A (MSYS2):
  - Install MSYS2, then `pacman -S --needed mingw-w64-ucrt-x86_64-toolchain mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-curl`
  - Build from a MinGW shell: `make`
- Option B (Visual Studio + vcpkg):
  - Install CMake and vcpkg. `vcpkg install curl`
  - Configure: `cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake`
  - Build: `cmake --build build --config Release`

Run From Project Root

- Use `make run ARGS='your query here'`
- Or call the binary directly at `./build/src/ani` after `make`

Output Formats

- Human-readable default view lists titles, counts, latest and next dates.
- JSON (`-j`) includes fields like `query`, `anime`, `manga`, with sub-objects for `latest` and `next` containing `number` and date (`YYYY-MM-DD`).

Caching

- Cache is initialized under a standard per-OS cache directory:
  - macOS: `~/Library/Caches/ani`
  - Linux: `$XDG_CACHE_HOME/ani` or `~/.cache/ani`
  - Windows: `%LOCALAPPDATA%\ani\Cache`
- Current release uses in-memory HTTP responses; on-disk cache APIs are present and will be used more in future versions.

Development Notes

- Code is C99 with strict warnings (`-Wall -Wextra -Werror -Wshadow -Wconversion -pedantic`).
- Single-line comments use C99 `//` style throughout the project.
- JSON parsing/writing by [yyjson] (vendored); HTTP by libcurl.
- Optional utf8proc is vendored but not required; it may be enabled later for normalization.

Troubleshooting

- If linking fails, ensure `libcurl` development package is installed and visible to your toolchain (e.g., run `pkg-config --libs libcurl`).
- If Windows build errors mention `sleep`, this repo implements Windows-compatible backoff using `Sleep()`; ensure you are compiling with the correct Windows headers (MSVC or MinGW).
- Network requests require internet and may be rate-limited; retry later or run with `-v`/`-vv` to see logs.

Editor Integration (clangd/VS Code)

- The build exports `compile_commands.json` and the Makefile symlinks it to the project root.
- If your editor shows header-not-found errors like `ani/cache.h file not found`, build once (`make` or `make debug`) so the database exists.
- With clangd, the included `.clangd` points to `build/` automatically. If you use a different build dir, update `.clangd` or invoke clangd with `--compile-commands-dir`.

License

- See LICENSE for details.

[yyjson]: https://github.com/ibireme/yyjson
