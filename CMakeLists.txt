cmake_minimum_required(VERSION 3.21)
project(qtwidgets_basic)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIRS ON)

find_package(Qt6 6.7 NO_MODULE REQUIRED COMPONENTS Widgets)

if(NOT DEFINED ENV{FLUTTER_ENGINE_FOLDER})
  message(FATAL_ERROR "Point env var FLUTTER_ENGINE_FOLDER to engine folder")
endif()

if(MSVC)
  set(FLUTTER_ENGINE_LIBRARY flutter_engine.dll.lib)
else()
  if(QT_EMBEDDER_AOT)
    set(FLUTTER_ENGINE_LIBRARY rel/libflutter_engine.so)
  else()
    set(FLUTTER_ENGINE_LIBRARY dbg/libflutter_engine.so)
  endif()
endif()

include_directories($ENV{FLUTTER_ENGINE_FOLDER})

add_executable(qtembedder main.cpp)

target_link_libraries(qtembedder Qt6::Widgets
                      $ENV{FLUTTER_ENGINE_FOLDER}/${FLUTTER_ENGINE_LIBRARY})

target_compile_definitions(qtembedder
                           PRIVATE FLUTTER_ICUDTL_DIR="${CMAKE_BINARY_DIR}")

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
