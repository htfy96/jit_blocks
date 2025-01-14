#pragma once
// NOLINTNEXTLINE(misc-include-cleaner)
#include <unistd.h>
// ---
#include <libgccjit.h>

#include "jit_blocks/jit_blocks.h"

/// An assertion macro that always evaluates at run time and aborts the program
/// if the condition is false.
#define JB_ASSERT(cond) \
  do { \
    if (!(cond)) { \
      (void)fprintf(stderr, \
                    "Assertion failed: %s at %s:%d\n", \
                    #cond, \
                    __FILE__, \
                    __LINE__); \
      abort(); \
    } \
  } while (0)

extern jit_blocks_context_builder_t context_builder;
