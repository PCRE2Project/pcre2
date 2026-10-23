#! /bin/sh

set -eu

git submodule update --init

if [ ! -e CMakeUserPresets.json ]; then
  echo '{ "version": 4, "include": [".devcontainer/CMakePresets.json"] }' > CMakeUserPresets.json
else
  echo 'Keeping existing CMakeUserPresets.json'
fi

# Ubuntu images include 'unminimize' for users who want to restore its full set
# of documentation. It is intentionally not run while building the image.

# Show the frontpage when a codespace is first opened. The devcontainer.json support
# for codespaces.openFiles is broken (it always opens the documents with code editor),
# so we use this hack.
code .github/README.md
