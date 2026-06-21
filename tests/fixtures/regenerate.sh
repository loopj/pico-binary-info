#!/usr/bin/env bash
set -euo pipefail

# Rebuild every fixture app in src/ across each supported board and binary type,
# copying the resulting binaries to bin/<dirname>-<board>-<type>.bin.
cd "$(dirname "$0")"
fixtures=$(pwd)

boards="pico pico2"
types="default copy_to_ram"

for dir in src/*/; do
  name=$(basename "$dir")
  cd "$fixtures/$dir"

  for board in $boards; do
    for type in $types; do
      build="build/$board-$type"
      cmake -B"$build" -DPICO_BOARD="$board" -DFIXTURE_BINARY_TYPE="$type"
      cmake --build "$build"
      cp "$build/$name.bin" "$fixtures/bin/$name-$board-$type.bin"
    done
  done
done
