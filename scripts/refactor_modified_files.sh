#!/usr/bin/env bash

if ! command -v clang-format &> /dev/null; then
  echo "Error: clang-format is not installed or not found in PATH."
  exit 1
fi

# Get the list of modified files
MODIFIED_FILES=$(git diff --name-only --diff-filter=M)
STAGED_FILES=$(git diff --cached --name-only)
UNTRACKED_FILES=$(git ls-files --others --exclude-standard)
ALL_FILES="$MODIFIED_FILES $STAGED_FILES $UNTRACKED_FILES"

# Filter the list to include only .cpp and .hpp files
files=()
for file in $ALL_FILES; do
  if [[ $file == *.cpp || $file == *.hpp ]]; then
    files+=("$file")
  fi
done

for file in "${files[@]}"; do
  if clang-format --style=file -i "$file"; then
    echo "Successfully formatted: $file"
  else
    echo "Failed to format: $file"
  fi
done