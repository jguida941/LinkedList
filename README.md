# Linked List Bid Loader

Linked_List is a C++20 console application that loads municipal bid data from CSV files into a custom singly linked list. The interactive menu lets you append manual entries, import a file, list bids in a formatted table, search, and remove by ID while timing key operations. The project bundles a lightweight CSV parser, Homebrew packaging, and helper scripts so you can build and run quickly on macOS, Linux, or Windows.

## Highlights
- Custom singly linked list with `Append`, `Prepend`, `Remove`, `Search`, formatted printing, and size tracking.
- CSV ingestion via `CSVparser` (included) that understands quoted fields, embedded commas, and header rows.
- Interactive menu with colorized, width-aware output and basic performance timing for load/search actions.
- Homebrew tap for one-command installs on macOS, plus convenience launch scripts for macOS (`run-mac.command`) and Windows (`run-win.bat`).
- Ready-to-run sample dataset: `cmake-build-debug/eBid_Monthly_Sales.csv` (used by default when you do not pass a path).

## Installation Options

### Homebrew Tap (recommended)
After publishing the tap repository:
```bash
brew tap jguida941/linkedlist
brew install linked-list
```
- To build from the latest commit instead of a tagged release: `brew install --HEAD linked-list`.
- The formula installs the `linked-list` executable into your PATH and copies the sample CSVs to `$(brew --prefix)/share/linked-list`.
- Run the app with the bundled sample data:
  ```bash
  linked-list "$(brew --prefix)/share/linked-list/eBid_Monthly_Sales.csv" 98109
  ```
- Both command-line arguments are optional; omit the CSV path and/or bid ID to fall back to the defaults shown in `LinkedList.cpp`.

### Clone and Build from Source
```bash
git clone https://github.com/jguida941/LinkedList.git
cd LinkedList
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/Linked_List cmake-build-debug/eBid_Monthly_Sales.csv 98109
```
- Requires CMake 3.16+ and a C++20-capable compiler (clang++ 11+, Apple Clang 13+, MSVC 2022, or g++ 10+).
- On Windows, use either Visual Studio with the “Desktop development with C++” workload or MinGW-w64.
- Omit the CSV/ID arguments to fall back to `cmake-build-debug/eBid_Monthly_Sales.csv` and bid ID `98109`.
- `cmake --build build --target run --config Release` executes the helper target defined in `CMakeLists.txt`.

## Repository Tour
- `LinkedList.cpp` – main application, menu loop, list implementation, console helpers.
- `CSVparser.cpp/.hpp` – self-contained CSV reader used by `loadBids`.
- `CMakeLists.txt` – builds a single executable target named `Linked_List` and defines a `run` helper target.
- `run-mac.command`, `run-win.bat` – double-clickable launchers that handle CMake builds and default arguments.
- `scripts/install-macos.sh` – optional installer/runner that can symlink a CLI into `/usr/local/bin`.
- `HomebrewFormula/linked-list.rb` – Homebrew tap formula for distributing source builds.
- `build/`, `cmake-build-debug/`, `dist/` – sample or generated artefacts; safe to remove when doing a clean build.

## Quick Start Reference

### macOS / Linux (Terminal)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/Linked_List cmake-build-debug/eBid_Monthly_Sales.csv 98109
```

### Windows (PowerShell or Developer Command Prompt)
**Visual Studio generator**
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
build/Release/Linked_List.exe cmake-build-debug/eBid_Monthly_Sales.csv 98109
```

**MinGW Makefiles**
```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
build/Linked_List.exe cmake-build-debug/eBid_Monthly_Sales.csv 98109
```

### Convenience Scripts
- `./run-mac.command [csvPath] [bidId]`
  - Resizes the terminal (configurable via `TARGET_COLS`/`TARGET_ROWS`), ensures CMake is installed, builds in `./build`, resolves CSV paths (including `cmake-build-debug/<file>`), and launches the executable. Keeps the window open when started via Finder.
- `run-win.bat [csvPath] [bidId]`
  - Tries the Visual Studio generator first, falls back to MinGW, validates the CSV path, resizes the console, and launches the built binary.
- `bash scripts/install-macos.sh [--link] [--run] [--csv path] [--bid id]`
  - Intended for a published repo: configures/builds Release binaries, optionally symlinks `/usr/local/bin/linked-list`, and can run the app in one shot.

## Command-Line Interface
```
Linked_List [csvPath] [bidId]
```
- `csvPath` – optional path to a CSV file containing the expected bid schema. Default: `cmake-build-debug/eBid_Monthly_Sales.csv` relative to the repo root.
- `bidId` – optional bid identifier used by the "Find" and "Remove" menu options. Default: `98109`.

### Menu flow
1. Enter a bid manually (appended to the list).
2. Load bids from CSV (timed in milliseconds/seconds).
3. Display all bids in a table with alignment, truncation, and compact mode for narrow terminals.
4. Find bid by the active `bidId` (timed in microseconds/seconds).
5. Remove bid by the active `bidId` if present.
9. Exit.

## Data Expectations
- CSV parser treats the first row as headers and supports quoted values with embedded commas.
- Columns used by `loadBids` (based on the bundled sample dataset):
  - `title` → column 0
  - `bidId` → column 1
  - `amount` → column 4 (e.g. `$123.45`, cleaned by `strToDouble`)
  - `fund` → column 8
- Loading adds bids to memory only; exiting discards changes. CSV write-back is not implemented.

## Terminal Experience
- Colors are controlled by the `COLOR_THEME` environment variable: `light` (default), `dark`, `mono`, or `none`.
- `run-mac.command` and `run-win.bat` honor `TARGET_COLS` / `TARGET_ROWS` to request a wider terminal for the table output.
- When width drops below ~80 columns, the app switches to a compact two-line layout so rows do not wrap.

## Development Notes
- The project defaults to Release builds in scripts for faster execution; use `-DCMAKE_BUILD_TYPE=Debug` if you prefer debug symbols.
- `LinkedList.cpp` uses `<chrono>` for higher-resolution search timing and `<time.h>` for the historical load timer to match the original coursework requirements.
- Memory management is manual; the destructor walks the list and frees nodes, so avoid copying `LinkedList` instances by value.
- The CSV parser exposes row access and mutation helpers beyond what the main program currently uses, which makes it easy to extend the CLI for editing or exporting data.

## Troubleshooting
- **"Failed to open" / "No Data"** – verify the CSV path; scripts will also attempt `cmake-build-debug/<file>` when run from the repo root.
- **Missing CMake** – install via `brew install cmake` (macOS), the Visual Studio installer (Windows), or your Linux package manager.
- **ANSI color codes visible** – set `COLOR_THEME=mono` (or `none`) or enable VT sequence support in the Windows console.
- **Executable not found after build** – clean `build/` and re-run CMake; multi-config generators output under `build/<Config>/Linked_List`.
- **`CSVparser` is bundled locally**; no external runtime dependencies are required.

## Known Limitations & Ideas
- The active bid ID is stored in memory and only changeable via the second command-line argument today.
- All changes are ephemeral; saving back to CSV or exporting a filtered list would be a natural extension.
- Searching is linear (`O(n)`); for very large datasets a different structure (hash map, balanced tree) might be warranted.
