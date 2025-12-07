# Linked List Bid Manager

A C++ console application that manages municipal bid data using a custom singly linked list. Built as a data structures project, it demonstrates linked list operations, CSV parsing, and terminal UI design.

## Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Usage](#usage)
- [Testing](#testing)
- [How It Works](#how-it-works)
- [Project Structure](#project-structure)
- [Requirements](#requirements)
- [Troubleshooting](#troubleshooting)

## Features

- **Custom linked list implementation** - Append, prepend, search, remove, and print operations
- **CSV file import** - Load thousands of bids from CSV files with quoted fields and embedded commas
- **Colorized terminal output** - Auto-detects dark/light terminal themes, adapts colors accordingly
- **Performance metrics** - Shows execution time for load and search operations
- **Responsive layout** - Adjusts output width based on terminal size
- **Unit tested** - Catch2 test suite covering linked list operations and input handling
- **Cross-platform** - Works on macOS, Linux, and Windows

## Quick Start

### Homebrew (macOS)

```bash
brew tap jguida941/linkedlist
brew install linked-list

# Run with the included sample data
linked-list "$(brew --prefix)/share/linked-list/eBid_Monthly_Sales.csv"
```

### Build from Source

**macOS / Linux:**
```bash
git clone https://github.com/jguida941/LinkedList.git
cd LinkedList
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/Linked_List
```

**Windows (Visual Studio):**
```powershell
git clone https://github.com/jguida941/LinkedList.git
cd LinkedList
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
.\build\Release\Linked_List.exe
```

**Windows (MinGW):**
```powershell
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
.\build\Linked_List.exe
```

### One-Click Scripts

For convenience, you can double-click these scripts to build and run automatically:
- **macOS:** `scripts/run-mac.command`
- **Windows:** `scripts/run-win.bat`

These scripts handle the CMake build process and locate the CSV files for you.

## Usage

```bash
./build/Linked_List [csv_path]
```

**Arguments:**
| Argument | Default | Description |
|----------|---------|-------------|
| `csv_path` | Auto-detected | Path to a CSV file with bid data |

The program automatically searches for `eBid_Monthly_Sales.csv` in common locations (`data/`, `../data/`, etc.), so you can run it without arguments from most directories.

### Menu

```
┌────────────────────────┐
│       BID SYSTEM       │
├────────────────────────┤
│ [1] Enter Bid          │
│ [2] Load Bids          │
│ [3] Show All           │
│ [4] Find Bid           │
│ [5] Remove Bid         │
├────────────────────────┤
│ [9] Exit               │
└────────────────────────┘
```

- **[1] Enter Bid** - Manually add a new bid (checks for duplicates)
- **[2] Load Bids** - Import bids from the CSV file
- **[3] Show All** - Display all loaded bids
- **[4] Find Bid** - Search for a bid by ID
- **[5] Remove Bid** - Delete a bid by ID
- **[9] Exit** - Quit the program

### Color Themes

The app detects your terminal background automatically. If colors look off, you can override:

```bash
# For dark terminals (bright colors)
export COLOR_THEME=dark

# For light terminals (darker colors)
export COLOR_THEME=light

# Disable colors entirely
export COLOR_THEME=mono
```

## Testing

The project includes unit tests using [Catch2](https://github.com/catchorg/Catch2). Tests cover:
- Whitespace trimming for user input
- Linked list operations (append, prepend, search, remove)
- Edge cases (empty lists, single elements, head/middle/tail removal)
- Integration tests for ID lookup with whitespace

### Running Tests

```bash
# Build with tests (enabled by default)
cmake -S . -B build
cmake --build build

# Run all tests
./build/tests

# Run specific test categories
./build/tests [trim]        # whitespace tests only
./build/tests [linkedlist]  # linked list tests only
./build/tests [integration] # integration tests only

# List all available tests
./build/tests --list-tests
```

### Disabling Tests

To build without tests (faster build):
```bash
cmake -S . -B build -DBUILD_TESTS=OFF
```

## How It Works

### Linked List

The core data structure is a singly linked list with head and tail pointers:

- **Append** - O(1) using tail pointer
- **Prepend** - O(1) by updating head
- **Search** - O(n) linear traversal
- **Remove** - O(n) to find, O(1) to unlink
- **Size tracking** - O(1) with counter variable

Each node stores a `Bid` struct with ID, title, fund name, and dollar amount.

### CSV Parser

The bundled `CSVparser` handles:
- Header row detection
- Quoted fields with embedded commas
- Dollar amounts with `$` symbols (stripped automatically)

The sample dataset (`eBid_Monthly_Sales.csv`) contains ~12,000 municipal bid records.

### Terminal Colors

Colors are set using ANSI escape codes with 256-color support:
- Dark mode: soft pastels that pop on dark backgrounds
- Light mode: darker shades that stay readable on white
- Mono mode: plain text for accessibility or piping to files

## Project Structure

```
LinkedList/
├── src/
│   ├── LinkedList.cpp      # Main program, linked list, menu loop
│   ├── CSVparser.cpp       # CSV file parser
│   └── CSVparser.hpp
├── tests/
│   └── test_linkedlist.cpp # Unit tests (Catch2)
├── data/
│   ├── eBid_Monthly_Sales.csv          # ~12,000 bid records
│   └── eBid_Monthly_Sales_Dec_2016.csv # Smaller sample
├── scripts/
│   ├── run-mac.command     # macOS build & run script
│   ├── run-win.bat         # Windows build & run script
│   └── install-macos.sh    # Optional CLI installer
├── HomebrewFormula/
│   └── linked-list.rb      # Homebrew tap formula
├── CMakeLists.txt          # CMake build configuration
├── LICENSE                 # MIT License
└── README.md
```

## Requirements

**Build tools:**
- CMake 3.16 or newer
- C++20 compatible compiler:
  - macOS: Xcode 13+ (Apple Clang 13+)
  - Linux: GCC 10+ or Clang 11+
  - Windows: Visual Studio 2022 or MinGW-w64

**Installing CMake:**
```bash
# macOS
brew install cmake

# Ubuntu/Debian
sudo apt install cmake build-essential

# Windows
# Install via Visual Studio Installer, or download from cmake.org
```

## Troubleshooting

| Problem | Fix |
|---------|-----|
| "Failed to open" CSV error | Make sure you're running from the project directory, or pass the full path to the CSV file |
| Colors look wrong | Try `export COLOR_THEME=dark` or `export COLOR_THEME=light` |
| Weird characters instead of box borders | Your terminal might not support Unicode. Try `export COLOR_THEME=mono` |
| Build fails with C++ errors | Make sure you have a C++20 compiler. On macOS, run `xcode-select --install` |
| Can't find the executable | It's in `build/Linked_List` (or `build/Release/Linked_List.exe` on Windows with VS) |

## License

MIT License - see [LICENSE](LICENSE) for details.