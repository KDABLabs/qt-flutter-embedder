# qt-flutter-embedder

Qt Flutter embedder for educational purposes and for playing with multi-window support.

Can be used to launch a 100% flutter app or to integrate flutter into an existing Qt app, in mixed mode.

## Already working

- Mouse
- HDPI
- Multi-Window
- OpenGLES
- OpenGL (with a patched flutter engine)
- QWindow
- Platform Channels (Dart calling C++)

## To-do

- Platform Channels (C++ calling Dart)
- Plugin registration
- Keyboard support
- Vulkan / Metal / D3D
- Integrate into QWidget or QtQuick

## Instructions

```bash
git submodule update --init
flutter prefetch --linux
cmake --preset=dev
cmake --build build-dev
./build_example.sh
./build-dev/qtembedder -m -e examples/default_counter_app
```

## Licensing

Licensed as GPL-3, feel free to contact us for other licensing options.
