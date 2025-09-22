#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Optionally resize the terminal window for readability
# Override with env: TARGET_COLS=140 TARGET_ROWS=40 ./run-mac.command
TARGET_COLS=${TARGET_COLS:-120}
TARGET_ROWS=${TARGET_ROWS:-35}
case "${TERM_PROGRAM:-}" in
  Apple_Terminal)
    # Small delay so the window exists before resizing
    sleep 0.2
    /usr/bin/osascript >/dev/null 2>&1 <<OSA
tell application "Terminal"
  try
    set number of columns of front window to ${TARGET_COLS}
    set number of rows of front window to ${TARGET_ROWS}
  end try
end tell
OSA
    ;;
  "iTerm.app")
    # iTerm2: set rows;cols
    printf '\e[8;%d;%dt' "$TARGET_ROWS" "$TARGET_COLS" || true
    ;;
  *) ;;
esac

if ! command -v cmake >/dev/null 2>&1; then
  echo "Error: cmake is not installed or not on PATH."
  echo "On macOS, install via: brew install cmake"
  read -rp $'\nPress Enter to close...' _
  exit 1
fi

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --config Release

CSV_PATH="${1:-eBid_Monthly_Sales.csv}"
BID_ID="${2:-98109}"

# If not found at project root, try CLion's debug folder bundled in this repo
if [[ ! -f "$SCRIPT_DIR/$CSV_PATH" && -f "$SCRIPT_DIR/cmake-build-debug/$CSV_PATH" ]]; then
  CSV_PATH="cmake-build-debug/$CSV_PATH"
fi

# Make CSV path absolute so it works regardless of current directory
if [[ "$CSV_PATH" != /* ]]; then
  CSV_PATH="$SCRIPT_DIR/$CSV_PATH"
fi

# Validate CSV existence early and give a clear message
if [[ ! -f "$CSV_PATH" ]]; then
  echo "Error: CSV file not found at: $CSV_PATH"
  echo "Pass a valid path as the first argument, e.g.:"
  echo "  $0 /full/path/to/eBid_Monthly_Sales.csv"
  read -rp $'\nPress Enter to close...' _
  exit 1
fi

# Locate the built binary across single- and multi-config generators
BIN_CANDIDATES=(
  "$BUILD_DIR/Linked_List"
  "$BUILD_DIR/Release/Linked_List"
  "$BUILD_DIR/Debug/Linked_List"
  "$BUILD_DIR/RelWithDebInfo/Linked_List"
  "$BUILD_DIR/MinSizeRel/Linked_List"
)

BIN_PATH=""
for cand in "${BIN_CANDIDATES[@]}"; do
  if [[ -x "$cand" ]]; then
    BIN_PATH="$cand"
    break
  fi
done

# Last resort: search within build dir
if [[ -z "$BIN_PATH" ]]; then
  BIN_PATH=$(find "$BUILD_DIR" -maxdepth 3 -type f -name Linked_List -perm -111 2>/dev/null | head -n1 || true)
fi

if [[ -z "${BIN_PATH}" || ! -x "${BIN_PATH}" ]]; then
  echo "Error: could not locate built executable in '$BUILD_DIR'."
  echo "Tried common paths and a short search."
  read -rp $'\nPress Enter to close...' _
  exit 1
fi

"$BIN_PATH" "$CSV_PATH" "$BID_ID"

# Keep Terminal open when double-clicked from Finder
read -rp $'\nDone. Press Enter to close...' _
