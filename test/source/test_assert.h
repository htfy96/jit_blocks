#pragma once

#include <stdio.h>  // IWYU pragma: keep
#include <stdlib.h>  // IWYU pragma: keep

/// An assertion macro that always evaluates at run time and aborts the program
/// if the condition is false.

// NOLINTBEGIN(concurrency-mt-unsafe)
#define JB_TEST_ASSERT(cond) \
  do { \
    if (!(cond)) { \
      (void)fprintf(stderr, \
                    "Assertion failed: %s at %s:%d\n", \
                    #cond, \
                    __FILE__, \
                    __LINE__); \
      exit(1); \
    } \
  } while (0)
// NOLINTEND(concurrency-mt-unsafe)