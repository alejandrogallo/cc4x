#!/usr/bin/env bash

set -eu

type -f clang-format || {
  echo "install clang-format-12"
  exit 1
}

fmt="echo {} && clang-format -i {} && echo {}"
set -x
for folder in src ibst; do
find \
  $folder/ \
  -type f \
  \( \
  -name '*.cxx' -or \
  -name '*.hpp' ! -name '*CLI11*' -or \
  -name '*.h' \
  \) \
  -exec sh -c "$fmt" \;
done
