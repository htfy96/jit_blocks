#include "common.h"

#include <assert.h>
// NOLINTNEXTLINE(misc-include-cleaner)
#include <unistd.h>
// --
#include <libgccjit.h>
#include <stdbool.h>

#include "jit_blocks/jit_blocks.h"

static gcc_jit_context* default_context_builder(void)
{
  gcc_jit_context* result = gcc_jit_context_acquire();
  assert(result != NULL);
  gcc_jit_context_set_bool_option(result, GCC_JIT_BOOL_OPTION_DEBUGINFO, true);
  gcc_jit_context_set_int_option(
      result, GCC_JIT_INT_OPTION_OPTIMIZATION_LEVEL, 3);
  return result;
}

jit_blocks_context_builder_t context_builder = default_context_builder;

jit_blocks_context_builder_t jit_blocks_get_easy_context_builder(void)
{
  return context_builder;
}

void jit_blocks_set_context_builder(jit_blocks_context_builder_t builder)
{
  context_builder = builder;
}
