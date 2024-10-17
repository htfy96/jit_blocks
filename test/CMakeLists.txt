cmake_minimum_required(VERSION 3.21)

project(jit_blocksTests LANGUAGES C)

include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(jit_blocks REQUIRED)
  enable_testing()
endif()

# ---- Tests ----

add_executable(jit_blocks_test source/jit_blocks_test.c)
target_link_libraries(jit_blocks_test PRIVATE jit_blocks::jit_blocks)
target_compile_features(jit_blocks_test PRIVATE c_std_99)

add_test(NAME jit_blocks_test COMMAND jit_blocks_test)

# ---- End-of-file commands ----

add_folders(Test)
