cmake_minimum_required(VERSION 3.21)

project(jit_blocksTests LANGUAGES C)

include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(jit_blocks REQUIRED)
  enable_testing()
endif()

function(add_jit_blocks_test test_name)
  add_executable(${test_name} source/${test_name}.c)
  target_link_libraries(${test_name} PRIVATE jit_blocks::jit_blocks)
  target_compile_features(${test_name} PRIVATE c_std_99)
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# ---- Tests ----

add_jit_blocks_test("jit_blocks_divide_test")
add_jit_blocks_test("jit_blocks_funccalls_test")
add_jit_blocks_test("jit_blocks_expr_test")

# ---- End-of-file commands ----

add_folders(Test)
