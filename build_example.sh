if [[ "$(uname)" == "Darwin" ]]; then
    FLUTTER_ARCH="_arm64"
    FLUTTER_HOST="macos"
else
    FLUTTER_ARCH=""
    FLUTTER_HOST="linux"
fi

cd examples/default_counter_app && \
flutter build ${FLUTTER_HOST} --debug --local-engine-src-path=$FLUTTER_ENGINE_FOLDER/.. --local-engine=host_debug_unopt${FLUTTER_ARCH} --local-engine-host=host_debug_unopt${FLUTTER_ARCH}
