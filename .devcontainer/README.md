# Development and CI containers

The published images use the official Debian 13 (Trixie) base, not a Microsoft
devcontainer image:

- `ghcr.io/pcre2project/pcre2-ci:latest`: the shared build/test/documentation tools;
  runs as root, suitable for a CI container.
- `ghcr.io/pcre2project/pcre2-interactive:latest`: the CI image plus terminal
  conveniences, debuggers, GitHub CLI and a `vscode` user with passwordless sudo.

Both images support amd64 and arm64. The interactive image inherits the actual
CI image layers, rather than installing a second copy of the common tools.
[devcontainer.json](devcontainer.json) pulls the interactive image; its editor
extensions and post-checkout setup remain separate from the image.

## Setup script

Run as root inside an official `debian:trixie` or `ubuntu:26.04` container, from
a checkout (or the files copied by the CI Dockerfile):

```sh
bash .devcontainer/setup.sh --env=ci
# Includes CI setup if it has not already run:
bash .devcontainer/setup.sh --env=interactive
```

Exactly one argument is required. Other distributions and architectures are
rejected. A completion marker allows the interactive layer to skip the already
installed common tooling. Rebuild the CI image to refresh that tooling; rerunning
the script against a completed installation is not an upgrade mechanism.

The common set covers GCC, Clang, CMake, Autotools, Meson, Zig, Bazelisk,
coverage, Valgrind, optional PCRE2 libraries, Python virtual environments and
documentation generation. It includes French and English UTF-8 locales.
Specialised CI configurations (old toolchains, cross-compilers, other operating
systems) are not all represented, and existing CI jobs do not yet use this image.

Website and manual gems come from [pages/Gemfile](../pages/Gemfile) and its
lockfile, installed system-wide and readable by both users. Node/npm supplies
Pagefind for the website build. Zig and Buildifier versions match the repository's
current requirements. clang-format 22 is retained for the editor; migrating the
formatting driver's clang-format 20 requirement is separate work.

Interactive additions include Vim, Delta, `xxd`, `batcat`, `fdfind`, `fzf`, Zsh
and tmux, without imposing a Git pager or shell preference. Users can bring
dotfiles through VS Code's `dotfiles.*` user settings or their personal GitHub
Codespaces settings. Ubuntu images include `unminimize`, but do not run it:
users can opt into its additional documentation with `sudo unminimize`.

## Build and test locally

Run from the repository root with Docker:

```sh
docker build --pull -f .devcontainer/Dockerfile.ci -t pcre2-ci:local .
docker build -f .devcontainer/Dockerfile \
  --build-arg CI_IMAGE=pcre2-ci:local -t pcre2-interactive:local .

git submodule update --init
docker run --rm -v "$PWD:/src:ro" -w /src pcre2-ci:local \
  bash .devcontainer/test.sh --env=ci
docker run --rm -v "$PWD:/src:ro" -w /src pcre2-interactive:local \
  bash .devcontainer/test.sh --env=interactive
```

To use the latest supported Ubuntu LTS instead, add
`--build-arg BASE_IMAGE=ubuntu:26.04` to the CI build. Build the interactive image
on that locally tagged CI image in the same way. For VS Code testing, temporarily
set the `image` in your local devcontainer configuration to
`pcre2-interactive:local`; do not commit that local override.

## Publishing

[The container workflow](../.github/workflows/containers.yml) runs weekly, on
relevant pull requests and main-branch changes, and manually. It performs fresh
builds and smoke tests on both distributions and both native architectures.
Tests exercise the locked website dependencies, Pagefind, Python virtual
environments and a three-width JIT-enabled PCRE2 build.

Only main-branch runs in `PCRE2Project/pcre2` push images. Debian images are
uploaded under per-run architecture tags, then both `latest` manifests are
published only after all four build/test jobs pass. Ubuntu is a compatibility
check, not a separately published image. The workflow uses `GITHUB_TOKEN` with
`packages: write`; no personal registry credentials are needed.

The first publication must be run before the new devcontainer can be pulled.
Ensure both GHCR packages are public so contributors can pull without registry
credentials. Retention of per-run tags and fixed release-image tags is a separate
maintainer policy. Existing containers do not update automatically; pull and
recreate them to consume a refreshed `latest`.
