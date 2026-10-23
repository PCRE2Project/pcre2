#!/bin/bash

set -euo pipefail

if [[ $# != 1 ]]; then
  echo "Usage: $0 --env=<ci|interactive>" >&2
  exit 2
fi
case "$1" in
  --env=ci) environment=ci ;;
  --env=interactive) environment=interactive ;;
  *) echo "Usage: $0 --env=<ci|interactive>" >&2; exit 2 ;;
esac
if [[ $EUID != 0 ]]; then
  echo "Run this script as root inside the container." >&2
  exit 1
fi

# shellcheck source=/dev/null
source /etc/os-release
case "$ID:$VERSION_ID" in
  debian:13 | ubuntu:26.04) ;;
  *) echo "Unsupported distribution: $PRETTY_NAME" >&2; exit 1 ;;
esac
arch=$(dpkg --print-architecture)
case "$arch" in
  amd64) zig_arch=x86_64 ;;
  arm64) zig_arch=aarch64 ;;
  *) echo "Unsupported architecture: $arch" >&2; exit 1 ;;
esac

source_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
export DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC
temp_dir=$(mktemp -d)
trap 'rm -rf -- "$temp_dir"' EXIT

download() {
  curl --fail --location --silent --show-error --retry 3 --output "$2" "$1"
}

install_ci() {
  apt-get update
  apt-get install -y --no-install-recommends ca-certificates curl gpg

  # Trixie's compiler is sufficient, but the editor uses clang-format 22.
  if [[ $ID == debian ]]; then
    download https://apt.llvm.org/llvm-snapshot.gpg.key "$temp_dir/llvm.asc"
    gpg --batch --yes --dearmor --output /usr/share/keyrings/llvm.gpg "$temp_dir/llvm.asc"
    echo 'deb [signed-by=/usr/share/keyrings/llvm.gpg] https://apt.llvm.org/trixie/ llvm-toolchain-trixie-22 main' \
      > /etc/apt/sources.list.d/llvm.list
    apt-get update
  fi

  apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build pkg-config autoconf automake libtool m4 \
    zlib1g-dev libbz2-dev libedit-dev libreadline-dev \
    llvm clang clang-tools clang-format-22 libclang-rt-dev valgrind lcov \
    ruby ruby-dev ruby-bundler nodejs npm \
    python3 python3-venv perl \
    git git-lfs openssh-client jq ripgrep shellcheck \
    file patch xz-utils bzip2 zip unzip locales man-db groff-base

  sed -i -e 's/^# *en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' \
    -e 's/^# *fr_FR.UTF-8 UTF-8/fr_FR.UTF-8 UTF-8/' /etc/locale.gen
  locale-gen

  local bundler_version
  bundler_version=$(awk '/^BUNDLED WITH$/ { getline; print $1 }' "$source_dir/pages/Gemfile.lock")
  gem install --no-document bundler --version "$bundler_version"
  BUNDLE_GEMFILE="$source_dir/pages/Gemfile" BUNDLE_FROZEN=true BUNDLE_PATH__SYSTEM=true \
    bundle "_${bundler_version}_" install

  python3 -m venv /opt/meson
  /opt/meson/bin/python -m pip install --no-cache-dir meson
  ln -s /opt/meson/bin/meson /usr/local/bin/meson

  local zig_download zig_url zig_sha256
  download https://ziglang.org/download/index.json "$temp_dir/zig-index.json"
  zig_download=$(jq -ec --arg arch "$zig_arch-linux" '.["0.16.0"][$arch]' "$temp_dir/zig-index.json")
  zig_url=$(jq -er '.tarball' <<< "$zig_download")
  zig_sha256=$(jq -er '.shasum' <<< "$zig_download")
  download "$zig_url" "$temp_dir/zig.tar.xz"
  echo "$zig_sha256  $temp_dir/zig.tar.xz" | sha256sum --check
  mkdir /opt/zig
  tar -xJf "$temp_dir/zig.tar.xz" -C /opt/zig --strip-components=1
  ln -s /opt/zig/zig /usr/local/bin/zig

  download "https://github.com/bazelbuild/bazelisk/releases/download/v1.30.0/bazelisk-linux-$arch" \
    /usr/local/bin/bazelisk
  download "https://github.com/bazel-contrib/buildtools/releases/download/v8.5.1/buildifier-linux-$arch" \
    /usr/local/bin/buildifier
  chmod 0755 /usr/local/bin/bazelisk /usr/local/bin/buildifier
  ln -s /usr/local/bin/bazelisk /usr/local/bin/bazel

  # Consumer tests must not accidentally find the distribution's PCRE2 headers.
  apt-get purge -y libpcre2-dev
  mkdir -p /var/lib/pcre2-container
  touch /var/lib/pcre2-container/ci-ready
}

install_interactive() {
  apt-get update
  apt-get install -y --no-install-recommends \
    aptitude bash-completion bat cmake-curses-gui fd-find fzf \
    gdb gh git-delta htop less lldb lsof manpages manpages-dev \
    nano ncdu procps psmisc rsync strace sudo tmux tree vim wget xxd zsh
  if [[ $ID == ubuntu ]]; then
    apt-get install -y --no-install-recommends unminimize
  fi

  if ! id vscode >/dev/null 2>&1; then
    if [[ $ID == ubuntu ]] && id ubuntu >/dev/null 2>&1; then
      usermod --login vscode --home /home/vscode --move-home ubuntu
      groupmod --new-name vscode ubuntu
    else
      useradd --create-home --uid 1000 --user-group --shell /bin/bash vscode
    fi
  fi
  usermod --shell /bin/bash vscode
  echo 'vscode ALL=(ALL) NOPASSWD:ALL' > /etc/sudoers.d/vscode
  chmod 0440 /etc/sudoers.d/vscode
  visudo --check --file /etc/sudoers.d/vscode
}

# The interactive image inherits this marker and all the CI tooling layers.
if [[ ! -f /var/lib/pcre2-container/ci-ready ]]; then
  install_ci
fi
if [[ $environment == interactive ]]; then
  install_interactive
fi
apt-get clean
find /var/lib/apt/lists -type f -delete
