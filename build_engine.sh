#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd ${SCRIPT_DIR}/3rdparty/flutter/
cp ./engine/scripts/standard.gclient .gclient
gclient sync # this step sometimes fails and needs to be repeated
./engine/src/flutter/tools/gn --unoptimized --no-goma --no-enable-unittests --disable-desktop-embeddings --no-build-glfw-shell --no-build-embedder-examples --no-stripped --enable-vulkan
ninja -C ./engine/src/out/host_debug_unopt
