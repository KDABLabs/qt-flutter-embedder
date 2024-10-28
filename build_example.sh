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

cd examples/default_counter_app && \
flutter build ${FLUTTER_HOST} --debug --local-engine-src-path=$FLUTTER_ENGINE_FOLDER/.. --local-engine=host_debug_unopt${FLUTTER_ARCH} --local-engine-host=host_debug_unopt${FLUTTER_ARCH}
