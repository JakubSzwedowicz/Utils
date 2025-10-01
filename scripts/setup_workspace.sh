#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -f /etc/debian_version ]]; then
  source "$SCRIPT_DIR/.debian/setup_workspace.sh"
elif [[ "$(uname -s)" == "Darwin" ]]; then
  source "$SCRIPT_DIR/.macos/setup_workspace.sh"
else
  echo "Unsupported OS!"
  exit 1
fi

git submodule update --init --recursive
cmake --workflow --preset all-debug --fresh