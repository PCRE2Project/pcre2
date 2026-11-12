#! /bin/sh

set -eu

git submodule update --init

(
  cd pages
  bundle config set --local path vendor/bundle
  bundle install
)

if [ ! -e CMakeUserPresets.json ]; then
  echo '{ "version": 4, "include": [".devcontainer/CMakePresets.json"] }' > CMakeUserPresets.json
else
  echo 'Keeping existing CMakeUserPresets.json'
fi
