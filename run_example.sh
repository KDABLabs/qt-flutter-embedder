if [ "$1" == "-n" ]; then
  NO_BUILD_FLUTTER=1
fi

if [ -z "$NO_BUILD_FLUTTER" ]; then
    cd examples/default_counter_app && \
    flutter build linux --debug --local-engine-src-path=$FLUTTER_ENGINE_FOLDER/.. --local-engine=host_debug_unopt --local-engine-host=host_debug_unopt && \
    cd ../..
fi

./build-dev/qtembedder examples/default_counter_app/
