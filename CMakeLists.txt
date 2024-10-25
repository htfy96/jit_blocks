cmake_minimum_required(VERSION 3.21)

include(cmake/prelude.cmake)

project(
    jit_blocks
    VERSION 0.1.0
    DESCRIPTION "Common JIT-powered building blocks for C using libgccjit"
    HOMEPAGE_URL "https://github.com/htfy96/jit_blocks"
    LANGUAGES C
)

include(cmake/variables.cmake)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(libgccjit REQUIRED)

find_library(MATH_LIBRARY m REQUIRED)

# ---- Declare library ----

add_library(
    jit_blocks_jit_blocks
    source/common.c
    source/divide.c
    source/funccalls.c
    source/expr.c
    source/dynswitch.c
)
target_link_libraries(jit_blocks_jit_blocks PUBLIC libgccjit::libgccjit PRIVATE ${MATH_LIBRARY})
add_library(jit_blocks::jit_blocks ALIAS jit_blocks_jit_blocks)

include(GenerateExportHeader)
generate_export_header(
    jit_blocks_jit_blocks
    BASE_NAME jit_blocks
    EXPORT_FILE_NAME export/jit_blocks/jit_blocks_export.h
)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(jit_blocks_jit_blocks PUBLIC JIT_BLOCKS_STATIC_DEFINE)
endif()

set_target_properties(
    jit_blocks_jit_blocks PROPERTIES
    C_VISIBILITY_PRESET hidden
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME jit_blocks
    OUTPUT_NAME jit_blocks
)

target_include_directories(
    jit_blocks_jit_blocks ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    jit_blocks_jit_blocks SYSTEM
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(jit_blocks_jit_blocks PUBLIC c_std_99)


# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT jit_blocks_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of jit_blocks"
  )
endif()

include(cmake/dev-mode.cmake)
