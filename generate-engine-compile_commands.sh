#!/bin/bash

# Generates compile_commands.json for hacking on the engine
# We don't pass --unoptimized as that would conflict with the build dir for the docker container where we build the engine

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd ${SCRIPT_DIR}/3rdparty/flutter/
cp ./engine/scripts/standard.gclient .gclient
./engine/src/flutter/tools/gn --no-goma --no-enable-unittests --disable-desktop-embeddings --no-build-glfw-shell --no-build-embedder-examples --no-stripped --enable-vulkan
cd engine/src/
ln -sf ./out/host_debug/compile_commands.json compile_commands.json

echo
echo "You may now run: zed ${SCRIPT_DIR}/3rdparty/flutter/engine/src"
