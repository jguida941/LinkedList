#!/usr/bin/env bash
set -euo pipefail

# macOS installer/builder for this project.
# Goals:
# - Let users install/run from Terminal with a one-liner (avoids Finder quarantine prompts)
# - Build into ./build and optionally symlink a CLI to /usr/local/bin
# - Keep dependencies minimal: Xcode CLT (clang) + CMake
#
# Usage (local checkout):
#   bash scripts/install-macos.sh [--link]
#
# Usage (remote one-liner after you publish):
#   bash <(curl -fsSL https://raw.githubusercontent.com/jguida941/LinkedList/main/scripts/install-macos.sh) --link
#
# Flags:
#   --link    Create/refresh a symlink at /usr/local/bin/linked-list (may prompt for sudo)
#   --run     Run the app after building
#   --csv <path>      CSV path to pass when running (default: cmake-build-debug/eBid_Monthly_Sales.csv)
#   --bid <id>        Bid ID to pass when running (default: 98109)

CSV_ARG="cmake-build-debug/eBid_Monthly_Sales.csv"
BID_ARG="98109"
DO_LINK=0
DO_RUN=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --link) DO_LINK=1; shift ;;
    --run)  DO_RUN=1;  shift ;;
    --csv)  CSV_ARG=${2:?}; shift 2 ;;
    --bid)  BID_ARG=${2:?}; shift 2 ;;
    *) echo "Unknown option: $1"; exit 2 ;;
  esac
done

need() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Error: missing dependency '$1'"
    echo "Install CMake via Homebrew: brew install cmake"
    exit 1
  fi
}

need cmake

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build"

echo "[1/3] Configuring build (Release)"
cmake -S "$REPO_ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release

echo "[2/3] Building executable"
cmake --build "$BUILD_DIR" --config Release

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

if [[ -z "$BIN_PATH" ]]; then
  echo "Error: built binary not found."
  exit 1
fi

if [[ $DO_LINK -eq 1 ]]; then
  TARGET_LINK="/usr/local/bin/linked-list"
  echo "[3/3] Linking CLI -> $TARGET_LINK"
  if [[ ! -d "/usr/local/bin" ]]; then
    echo "Creating /usr/local/bin (requires sudo)"
    sudo mkdir -p /usr/local/bin
  fi
  if [[ -L "$TARGET_LINK" || -f "$TARGET_LINK" ]]; then
    sudo rm -f "$TARGET_LINK"
  fi
  sudo ln -s "$BIN_PATH" "$TARGET_LINK"
  echo "Installed: $TARGET_LINK"
fi

if [[ $DO_RUN -eq 1 ]]; then
  CSV_PATH="$CSV_ARG"
  if [[ "$CSV_PATH" != /* ]]; then
    CSV_PATH="$REPO_ROOT/$CSV_PATH"
  fi
  if [[ ! -f "$CSV_PATH" ]]; then
    echo "Warning: CSV not found at $CSV_PATH"
  fi
  echo "Running: $BIN_PATH $CSV_PATH $BID_ARG"
  "$BIN_PATH" "$CSV_PATH" "$BID_ARG"
fi

echo "Done."
