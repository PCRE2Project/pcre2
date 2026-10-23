#!/bin/bash

set -euo pipefail

if [[ $# != 1 || ( $1 != --env=ci && $1 != --env=interactive ) ]]; then
  echo "Usage: $0 --env=<ci|interactive>" >&2
  exit 2
fi
environment=${1#--env=}
source_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
temp_dir=$(mktemp -d)
trap 'rm -rf -- "$temp_dir"' EXIT

for tool in gcc g++ clang clang-format-22 llvm-cov llvm-profdata \
  cmake ctest ninja pkg-config autoconf automake libtoolize \
  valgrind lcov ruby bundle node npm npx python3 perl git git-lfs \
  jq rg shellcheck man nroff meson zig bazelisk buildifier; do
  command -v "$tool"
done
clang-format-22 --version
zig version | grep -Fx 0.16.0
buildifier --version | grep -F 8.5.1
locale -a | grep -Fx fr_FR.utf8
if [[ -e /usr/include/pcre2.h ]]; then
  echo "The distribution's PCRE2 development headers must not be installed." >&2
  exit 1
fi

python3 -m venv "$temp_dir/venv"
"$temp_dir/venv/bin/python" -m pip --version
BUNDLE_GEMFILE="$source_dir/pages/Gemfile" BUNDLE_FROZEN=true bundle check
ruby -rasciidoctor -rasciidoctor-html5s -rrouge -rnokogiri -runicode/name -e 'puts "Documentation gems available"'
ruby "$source_dir/maint/BuildPages.rb" --base-url https://example.org/pcre2/ \
  --destination "$temp_dir/site"
test -s "$temp_dir/site/index.html"
test -s "$temp_dir/site/pagefind/pagefind.js"

if [[ $environment == interactive ]]; then
  test "$(id -un)" = vscode
  test -w "$HOME"
  sudo -n true
  for tool in vim delta xxd batcat fdfind fzf gh gdb lldb tmux zsh; do
    command -v "$tool"
  done
fi

cmake -S "$source_dir" -B "$temp_dir/build" -G Ninja \
  -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug \
  -DPCRE2_BUILD_PCRE2_16=ON -DPCRE2_BUILD_PCRE2_32=ON \
  -DPCRE2_SUPPORT_JIT=ON -DPCRE2_SUPPORT_LIBREADLINE=ON \
  -DPCRE2_SUPPORT_VALGRIND=ON
cmake --build "$temp_dir/build" --parallel 2
ctest --test-dir "$temp_dir/build" --output-on-failure --parallel 2
