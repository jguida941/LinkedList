#!/usr/bin/env bash
set -euo pipefail

# Prepare a Homebrew Tap repo folder locally.
# Result: dist/homebrew-linkedlist/Formula/linked-list.rb
# Next steps (manual): create GitHub repo jguida941/homebrew-linkedlist and push.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
TAP_DIR="$REPO_ROOT/dist/homebrew-linkedlist/Formula"

mkdir -p "$TAP_DIR"
cp -f "$REPO_ROOT/HomebrewFormula/linked-list.rb" "$TAP_DIR/linked-list.rb"

cat >&2 <<MSG
Tap folder ready at: dist/homebrew-linkedlist

To publish:
  1) Create repo: https://github.com/jguida941/homebrew-linkedlist (public)
  2) From this project root, run:
       cd dist/homebrew-linkedlist
       git init
       git add .
       git commit -m "Add linked-list formula"
       git branch -M main
       git remote add origin git@github.com:jguida941/homebrew-linkedlist.git
       git push -u origin main

Users can then install via:
  brew tap jguida941/linkedlist
  brew install linked-list

If you prefer HTTPS remote, replace the git@ URL accordingly.
MSG

echo "Done."

