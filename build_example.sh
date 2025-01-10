#!/bin/bash

if [[ "$(uname)" == "Darwin" ]]; then
    FLUTTER_HOST="macos"
else
    FLUTTER_HOST="linux"
fi

if uname -m | grep "arm64"; then
    FLUTTER_ARCH="_arm64"
else
    FLUTTER_ARCH=""
fi

cd examples/default_counter_app && flutter pub get && \
flutter build ${FLUTTER_HOST} --debug
