# This file is part of qt-flutter-embedder.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com> Author: Sérgio Martins <sergio.martins@kdab.com>
# SPDX-License-Identifier: GPL-3.0-only Contact KDAB at <info@kdab.com> for
# commercial licensing options.

cmake_minimum_required(VERSION 3.21)
project(qtwidgets_basic)

option(DEVELOPER_BUILD "Enable DEVELOPER_BUILD (tests, helper code, etc)" OFF)
option(USE_PREBUILT_ENGINE "Uses the engine in engine_binaries/ instead of building it (requires running download_engine.sh)" ON)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/flutter/README.md")
  message(FATAL_ERROR "Missing required Flutter submodule. Please run: git submodule update --init")
endif()

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIRS ON)

find_package(Qt6 6.6 NO_MODULE REQUIRED COMPONENTS Widgets OpenGL)

if(MSVC)
  # Windows is not tested, this probably doesn't work
  set(FLUTTER_ENGINE_LIBRARY flutter_engine.lib)
else()

  set(ENGINE_FILENAME libflutter_engine${CMAKE_SHARED_LIBRARY_SUFFIX})

  if (USE_PREBUILT_ENGINE)
    set(HELPER_SCRIPT download_engine.sh)
    if(QT_EMBEDDER_AOT)
        set(FLUTTER_ENGINE_LIBRARY
            ${CMAKE_SOURCE_DIR}/engine_binaries/rel/${ENGINE_FILENAME}
        )
    else()
        set(FLUTTER_ENGINE_LIBRARY
            ${CMAKE_SOURCE_DIR}/engine_binaries/dbg_unopt1/${ENGINE_FILENAME}
        )
    endif()
  else()
    set(HELPER_SCRIPT build_engine.sh)
    if(QT_EMBEDDER_AOT)
        # We're only building engine for debug, for now
        message(FATAL_ERROR "AOT mode is not supported for building the engine from source. Please use USE_PREBUILT_ENGINE=ON with QT_EMBEDDER_AOT=ON or disable AOT mode.")
    else()
        set(FLUTTER_ENGINE_LIBRARY
            ${CMAKE_SOURCE_DIR}/3rdparty/flutter/engine/src/out/host_debug_unopt/${ENGINE_FILENAME}
        )
    endif()
  endif()
endif()

message("FLUTTER_ENGINE_LIBRARY=${FLUTTER_ENGINE_LIBRARY}}")

if(NOT EXISTS ${FLUTTER_ENGINE_LIBRARY})
  message(
    FATAL_ERROR
      "Could not find ${FLUTTER_ENGINE_LIBRARY}. Did you run ${HELPER_SCRIPT} ?"
  )
endif()

if(ENABLE_ASAN)
  add_compile_options(-fsanitize=address -fsanitize=undefined)
  add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

add_library(
  flutter_common_client_wrapper STATIC
  3rdparty/flutter/engine/src/flutter/shell/platform/common/client_wrapper/plugin_registrar.cc
  3rdparty/flutter/engine/src/flutter/shell/platform/common/client_wrapper/core_implementations.cc
  3rdparty/flutter/engine/src/flutter/shell/platform/common/client_wrapper/standard_codec.cc
  3rdparty/flutter/engine/src/flutter/shell/platform/common/client_wrapper/engine_method_result.cc
  3rdparty/flutter/engine/src/flutter/shell/platform/common/client_wrapper/engine_method_result.cc
  3rdparty/flutter/engine/src/flutter/shell/platform/common/incoming_message_dispatcher.cc)

target_include_directories(
  flutter_common_client_wrapper
  PUBLIC 3rdparty/flutter/engine/src/flutter/shell/platform/common/public
         3rdparty/flutter/engine/src/flutter/shell/platform/embedder/
         3rdparty/flutter/engine/src/flutter/shell/platform/common
         3rdparty/flutter/engine/src/flutter/shell/platform/common/client_wrapper
         3rdparty/flutter/engine/src/flutter/shell/platform/common/client_wrapper/include/
         3rdparty/flutter/engine/src/flutter/..)

add_executable(qtembedder main.cpp src/Embedder.cpp src/FlutterWindow.cpp
                          src/3rdparty/flutter/glfw_shell.cpp)

target_link_libraries(qtembedder Qt6::Widgets Qt6::OpenGL Qt6::GuiPrivate
                      ${FLUTTER_ENGINE_LIBRARY} flutter_common_client_wrapper)

if(NOT APPLE)
  target_link_libraries(qtembedder EGL)
endif()

target_compile_definitions(qtembedder
                           PRIVATE FLUTTER_ICUDTL_DIR="${CMAKE_BINARY_DIR}")

if(DEVELOPER_BUILD)
  target_compile_definitions(qtembedder PRIVATE DEVELOPER_BUILD)
endif()

# ICU
find_program(FLUTTER_EXE flutter REQUIRED)
get_filename_component(FLUTTER_SDK_PATH ${FLUTTER_EXE} PATH)
message(STATUS "Flutter SDK: ${FLUTTER_SDK_PATH}")
file(GLOB_RECURSE ICUDTL "${FLUTTER_SDK_PATH}/icudtl.dat")
message(STATUS "flutter icudtl.dat file: ${ICUDTL}")
if(ICUDTL)
  add_custom_command(
    TARGET qtembedder
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${ICUDTL} ${CMAKE_BINARY_DIR}
    COMMENT "Copying icudtl.dat to ${CMAKE_BINARY_DIR}")
else()
  message(FATAL_ERROR "icudtl.dat not found! Try running: flutter precache --linux")
endif()

if(QT_EMBEDDER_AOT)
  target_compile_definitions(qtembedder PRIVATE FLUTTER_AOT)
endif()

add_custom_target(
  flutter_precache
  COMMAND flutter config --no-analytics
  COMMAND flutter precache --linux
  COMMENT "Configuring Flutter: disabling analytics and precaching Linux dependencies"
  VERBATIM
)

add_dependencies(qtembedder flutter_precache)
