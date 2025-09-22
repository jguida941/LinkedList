**Linked_List (C++ Linked List + CSV Loader)**
- Console app that demonstrates a singly linked list by loading “bids” from a CSV file, with a simple menu to add, list, find, and remove bids. Includes a small CSV parser library and colored, aligned console output.

**File Tree**
- `CMakeLists.txt` — CMake build config (C++20, single executable `Linked_List`).
- `LinkedList.cpp` — Main program with:
  - `struct Bid { bidId, title, fund, amount }`
  - `class LinkedList` (singly linked list with `Append`, `Prepend`, `Remove`, `Search`, `PrintList`, `Size`)
  - CSV and console helpers: `loadBids`, `getBid`, `displayBid`, `displayBidCompact`, `waitForEnter`, `strToDouble`
  - `main` with interactive menu + timing output
- `CSVparser.hpp/.cpp` — Lightweight CSV reader (header parsing, quoted cells, row access, add/delete rows, write‑back when reading from a file).
- `cmake-build-debug/` — Example build folder (CLion). Contains the bundled sample CSVs, e.g., `eBid_Monthly_Sales.csv`.
- `run-mac.command` — Double‑clickable macOS script to build and run.
- `run-win.bat` — Double‑clickable Windows script to build and run (MSVC or MinGW).

Note: For fresh builds, prefer a clean `build/` directory as shown in the instructions below. The `cmake-build-debug/` folder comes from CLion and is safe to ignore.

**What It Does**
- Loads bids from a CSV into a custom singly linked list and lets you interact via a menu:
  - Enter a bid manually and append it to the list.
  - Load bids from CSV and measure load time (milliseconds/seconds).
- Display all bids in a table‑style, colored, aligned format.
  - Output adapts to terminal width, truncates Title with ellipses, and switches to a compact two‑line layout on very narrow windows to avoid wrapping.
  - Find a bid by `bidId` and measure search time (microseconds/seconds).
  - Remove a bid by `bidId` if present.

**Usage Basics**
- Run with optional arguments: `./Linked_List [csvPath] [bidId]`
  - If omitted, defaults to `eBid_Monthly_Sales.csv` and `98109`.
  - The included sample CSVs live under `cmake-build-debug/`.
  - Menu options “Find Bid” and “Remove Bid” use the `bidId` value given on startup (second arg); change it by passing a different second argument or restarting.

**Current Limitations (School Assignment Defaults)**
- Hardcoded search key: the app is set up to look for bid ID `98109` by default. This key lives in memory and is only changeable via the second command‑line argument when launching. The menu does not currently prompt for a different ID.
- In‑memory only: the linked list is an in‑memory structure. “Enter a Bid” adds to memory for the current session only; it does not persist to disk. Loading a CSV reads data; exiting discards any changes.
- CSV is read‑only today: there is no write‑back of edits or new bids to the CSV yet.

Planned enhancements
- Add a menu flow to change the “active” Bid ID used by Find/Remove without restarting.
- Support saving changes: append new bids and/or write out a full updated CSV (new output file to avoid clobbering the original by default).
- Optional data directory (e.g., `assets/`) with explicit read/write paths controlled by command‑line flags.

**CSV Format Assumptions**
- Uses specific columns by index (based on the eBid dataset):
  - `title` = column 0
  - `bidId` = column 1
  - `amount` = column 4 (string like `$123.45`, converted with `strToDouble`)
  - `fund`   = column 8
- Quoted fields and embedded commas are handled by `CSVparser`.

**Code Tour**
- `struct Bid`
  - Fields: `bidId` (string), `title` (string), `fund` (string), `amount` (double).

- `class LinkedList`
  - Internals: `Node { Bid bid; Node* next; }`, and `head`, `tail`, `size`.
  - `Append(const Bid&)`: add to tail in O(1).
  - `Prepend(const Bid&)`: add to head in O(1).
  - `Remove(const string& bidId)`: unlink first matching node, update `tail` if needed.
  - `Search(const string& bidId) const`: linear scan; returns empty `Bid{}` if not found.
  - `PrintList() const`: iterate and pretty‑print every `Bid`.
  - `Size() const`: current element count.

- Console helpers
  - `displayBid`, `displayBidCompact`: colorized, aligned outputs; compact confirmation after add avoids overly wide rows.
  - `waitForEnter`: pause prompt between menu actions.
  - `getBid`: prompts for fields and parses amount.
  - `strToDouble`: strips a character (e.g., `$`) and converts to `double`.

- CSV loader
  - `loadBids(csvPath, LinkedList*)`: reads rows with `csv::Parser`, maps required columns, appends bids.
  - Exceptions from `CSVparser` are caught and printed.

- `main(int argc, char* argv[])`
  - Args: `[1]=csvPath` (optional), `[2]=bidId` (optional, default `98109`).
  - Menu options: 1) Enter 2) Load 3) Display 4) Find 5) Remove 9) Exit.
  - Measures load time with `clock()` and search time with `high_resolution_clock`.

**Build and Run (macOS)**
- Prereqs: Xcode CLT or `clang++`, and CMake 3.16+.
- Install CMake if needed: `brew install cmake`.
- Commands:
  - `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
  - `cmake --build build --config Release`
  - `./build/Linked_List cmake-build-debug/eBid_Monthly_Sales.csv 98109`

**Easy macOS Launch**
- First time only: `chmod +x run-mac.command` (make it executable).
- Double‑click `run-mac.command` in Finder, or run:
  - `./run-mac.command [csvPath] [bidId]`
- The script builds into `build/` and runs the app.
- CSV path handling: if a relative path is given and not found at repo root, it auto‑tries `cmake-build-debug/<csv>`. The script makes the CSV path absolute before launch and validates existence (clear error if missing).
- Window size: auto‑resizes the Terminal window to 120x35 for readability. Override with env vars: `TARGET_COLS=140 TARGET_ROWS=40 ./run-mac.command`.

**Build and Run (Windows)**
- Option A: Visual Studio (MSVC)
  - Install Visual Studio with C++ Desktop workload + CMake.
  - From “x64 Native Tools Command Prompt for VS”:
    - `cmake -S . -B build -G "Visual Studio 17 2022" -A x64`
    - `cmake --build build --config Release`
    - `build/Release/Linked_List.exe cmake-build-debug/eBid_Monthly_Sales.csv 98109`

- Option B: MinGW-w64 (gcc)
  - Install MinGW‑w64 and CMake; ensure `g++` on PATH.
  - `cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release`
  - `cmake --build build --config Release`
  - `build/Linked_List.exe cmake-build-debug/eBid_Monthly_Sales.csv 98109`

**Easy Windows Launch**
- Double‑click `run-win.bat` in Explorer, or from Command Prompt:
  - `run-win.bat [csvPath] [bidId]`
- The script tries Visual Studio 2022 first, then falls back to MinGW Makefiles.
- CSV path handling: resolves relative paths against the repo folder, auto‑tries `cmake-build-debug\<csv>`, validates existence, and passes an absolute path to the program.
- Window size: sets the console to 120x35. Override with env vars (before running the BAT): `set TARGET_COLS=140` and `set TARGET_ROWS=40`.

**Notes and Tips**
- If you run from the repo root and don’t copy a CSV there, pass `cmake-build-debug/eBid_Monthly_Sales.csv` as the path (or move a CSV into the root).
- Colors: high‑contrast palette for light terminals by default. Override with env `COLOR_THEME`:
  - `COLOR_THEME=light` (default): bold, dark tones (teal/green/blue) for white backgrounds.
  - `COLOR_THEME=dark`: standard bright ANSI colors for dark backgrounds.
  - `COLOR_THEME=mono` or `none`: disable colors.
  Most modern terminals support ANSI/256 colors. If your console shows raw "\033[..m" sequences, enable ANSI/VT or use a modern terminal.
- The list operations here are linear except append/prepend, which are O(1). For large datasets, searching is O(n).

**Troubleshooting**
- “Failed to open” or “No Data” errors: check the CSV path and file permissions.
- Linker/compile errors: ensure you’re using C++20 (`CMakeLists.txt` already sets `CMAKE_CXX_STANDARD 20`).
- Windows path issues: wrap paths with quotes, use backslashes or forward slashes consistently.
- “Hardcoded path” concerns: launch scripts resolve the CSV path to an absolute path and validate it, then pass that absolute path to the program. They also fall back to the bundled `cmake-build-debug/<csv>` for convenience after a fresh clone.
- macOS: Finder blocked the `.command` file
  - Right‑click `run-mac.command` → Open → Open.
  - Or remove quarantine and ensure execute bit:
    - `xattr -dr com.apple.quarantine ./run-mac.command`
    - `chmod +x ./run-mac.command`
  - You can also run it directly from Terminal to bypass Finder’s Gatekeeper prompts: `./run-mac.command`.

**License/Attribution**
- CSV parser in `CSVparser.hpp/.cpp` is bundled with this project. Main application adapted from an academic exercise to demonstrate linked lists and basic CSV ingestion.

**Simple CMake Run (macOS/Linux)**
- Configure once: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
- Run via target: `cmake --build build --target run --config Release`
  - Uses `cmake-build-debug/eBid_Monthly_Sales.csv` and `98109` by default.
  - The app is interactive; press keys in the menu.
  - Quick non‑interactive smoke test: `printf "2\n3\n9\n" | ./build/Linked_List cmake-build-debug/eBid_Monthly_Sales.csv 98109`

**Simple CMake Run (Windows)**
- After configuring a build folder (MSVC or MinGW), run the target:
  - `cmake --build build --target run --config Release`
  - It launches the interactive app with the bundled sample CSV.
