# jit_blocks

JitBlocks is a C library built on the top of `libgccjit` that provides common building blocks for JIT-powered code.

This overview page is best viewed from [the rendered Doxygen page](https://htfy96.github.io/jit_blocks/).

# API Overview

At a high level, there are two types of API interfaces:

- **Easy** APIs. These APIs create a new compilation context using `::jit_blocks_get_easy_context_builder`, builds a function in it, frees the compilation context, and return the `gcc_jit_result` and function pointer to end users. It's user's responsibility to call `gcc_jit_result_release` on the returned `gcc_jit_result`. Users could replace the default context builder with `::jit_blocks_set_easy_context_builder`. Example:

```c
  gcc_jit_result* result = NULL;
  jit_blocks_divide_by_k_int_t divide_by_42 =
      jit_blocks_build_divide_by_k_int(42, &result);
  assert(divide_by_42 != NULL);
  assert(divide_by_42(100) == 2);
  gcc_jit_result_release(result);
```

- Low-level APIs. These APIs are suffixed with `_aux` and accept an additional `gcc_jit_context* custom_ctx` parameter. It **does not free the passed-in custom_ctx**. Users should free both `custom_ctx` and `result` at the end of call, as `custom_ctx`'s state is unspecified after call and should not be reused. Example:

```c
  gcc_jit_result* result = NULL;
  gcc_jit_context* ctx = gcc_jit_context_acquire();
  // set arbitrary option for this ctx
  jit_blocks_divide_by_k_int_t divide_by_42 =
      jit_blocks_build_divide_by_k_int_aux(42, ctx, &result);
  assert(divide_by_42 != NULL);
  assert(divide_by_42(100) == 2);
  gcc_jit_result_release(result);
  gcc_jit_context_release(ctx);
```

## Easy context builder API

See @ref context_builder

## divide-by API builder

Builds `divide_by_constant` functions. These division functions are often faster than writing `runtime_var / runtime_var` expressions, as compiler could utilize the known divisor and convert the division into multiplications. Provides similar speedup to [libdivide](https://libdivide.com/).

See @ref divide.

## Function Calls builder

Builds a function that calls all specified function pointers in order. Compared to calling function vectors at runtime, it's more branch-predictor-friendly and allows more speculative execution.

See @ref funccalls

## Expression Engine

Builds a stack-based arithmetic expression interpreter.

See @ref expr

## Dynamic Switch builder

Builds a dynamic `switch (val) { case A: ... }` block. Useful when the dispatch table is only known at runtime.

See @ref dynswitch

# Using this library

For CMake users, the recommended way is to add this project either as a git submodule, or download via [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) or [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake), and then add these lines to your CMakeLists.txt:

```cmake
add_subdirectory("jit_blocks_dir")
target_link_libraries(YOUR_LIB PRIVATE jit_blocks::jit_blocks)
```

It should automatically take care of all include directories and linking flags.

Alternatively, you can also write Shell scripts following [BUILDING](./BUILDING.md) instructions, install them into a local directory, and then manually include the generated headers and built library.

# Building and installing

See the [BUILDING](./BUILDING.md) document.

# Contributing

See the [CONTRIBUTING](./CONTRIBUTING.md) document.

# Licensing

Apache v2
