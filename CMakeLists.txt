# This file is part of qt-flutter-embedder.
#
# SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group
# company <info@kdab.com> Author: Sérgio Martins <sergio.martins@kdab.com>
# SPDX-License-Identifier: GPL-3.0-only Contact KDAB at <info@kdab.com> for
# commercial licensing options.

cmake_minimum_required(VERSION 3.21)
project(qtwidgets_basic)

option(DEVELOPER_BUILD "Enable DEVELOPER_BUILD (tests, helper code, etc)" OFF)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIRS ON)

find_package(Qt6 6.7 NO_MODULE REQUIRED COMPONENTS Widgets OpenGL)

if(NOT DEFINED ENV{FLUTTER_ENGINE_FOLDER})
  message(FATAL_ERROR "Point env var FLUTTER_ENGINE_FOLDER to engine folder")
endif()

if(MSVC)
  set(FLUTTER_ENGINE_LIBRARY flutter_engine.lib)
else()
  if(EXISTS "$ENV{FLUTTER_ENGINE_FOLDER}/rel/")
    if(QT_EMBEDDER_AOT)
      set(FLUTTER_ENGINE_LIBRARY
          rel/libflutter_engine.${CMAKE_SHARED_LIBRARY_SUFFIX})
    else()
      set(FLUTTER_ENGINE_LIBRARY
          dbg_unopt/libflutter_engine.${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
    set(FLUTTER_ENGINE_INCLUDE_DIR $ENV{FLUTTER_ENGINE_FOLDER})
  else()
    if(APPLE)
      set(BUILD_FOLDER_NAME host_debug_unopt_arm64)
    else()
      set(BUILD_FOLDER_NAME host_debug_unopt)
    endif()

    set(FLUTTER_ENGINE_LIBRARY
        ../out/${BUILD_FOLDER_NAME}/libflutter_engine${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
    set(FLUTTER_ENGINE_INCLUDE_DIR
        $ENV{FLUTTER_ENGINE_FOLDER}/../out/${BUILD_FOLDER_NAME})
  endif()
endif()

message(
  "FLUTTER_ENGINE_LIBRARY=$ENV{FLUTTER_ENGINE_FOLDER}/${FLUTTER_ENGINE_LIBRARY}"
)

include_directories(${FLUTTER_ENGINE_INCLUDE_DIR})

if(ENABLE_ASAN)
  add_compile_options(-fsanitize=address -fsanitize=undefined)
  add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

add_library(
  flutter_common_client_wrapper STATIC
  $ENV{FLUTTER_ENGINE_FOLDER}/shell/platform/common/client_wrapper/plugin_registrar.cc
  $ENV{FLUTTER_ENGINE_FOLDER}/shell/platform/common/client_wrapper/core_implementations.cc
  $ENV{FLUTTER_ENGINE_FOLDER}/shell/platform/common/client_wrapper/standard_codec.cc
  $ENV{FLUTTER_ENGINE_FOLDER}/shell/platform/common/client_wrapper/engine_method_result.cc
  $ENV{FLUTTER_ENGINE_FOLDER}/shell/platform/common/client_wrapper/engine_method_result.cc
  $ENV{FLUTTER_ENGINE_FOLDER}/shell/platform/common/incoming_message_dispatcher.cc
)

target_include_directories(
  flutter_common_client_wrapper
  PUBLIC
    "$ENV{FLUTTER_ENGINE_FOLDER}/shell/platform/common/public"
    $ENV{FLUTTER_ENGINE_FOLDER}/shell/platform/embedder/
    $ENV{FLUTTER_ENGINE_FOLDER}/shell/platform/common
    $ENV{FLUTTER_ENGINE_FOLDER}/shell/platform/common/client_wrapper
    $ENV{FLUTTER_ENGINE_FOLDER}/shell/platform/common/client_wrapper/include/
    "$ENV{FLUTTER_ENGINE_FOLDER}/..")

add_executable(qtembedder main.cpp src/Embedder.cpp src/FlutterWindow.cpp
                          src/3rdparty/flutter/glfw_shell.cpp)

target_link_libraries(
  qtembedder Qt6::Widgets Qt6::OpenGL Qt6::GuiPrivate
  $ENV{FLUTTER_ENGINE_FOLDER}/${FLUTTER_ENGINE_LIBRARY}
  flutter_common_client_wrapper)

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
message(STATUS "Flutter SDK: ${FLUTTER_EXE}")
get_filename_component(FLUTTER_SDK_PATH ${FLUTTER_EXE} PATH)
file(GLOB_RECURSE ICUDTL "${FLUTTER_SDK_PATH}/icudtl.dat")
message(STATUS "flutter icudtl.dat file: ${ICUDTL}")
if(ICUDTL)
  add_custom_command(
    TARGET qtembedder
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${ICUDTL} ${CMAKE_BINARY_DIR}
    COMMENT "Copying icudtl.dat to ${CMAKE_BINARY_DIR}")
else()
  message(FATAL_ERROR "icudtl.dat not found!")
endif()

if(QT_EMBEDDER_AOT)
  target_compile_definitions(qtembedder PRIVATE FLUTTER_AOT)
endif()
