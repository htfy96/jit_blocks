#include "jit_blocks/jit_blocks.h"

#include <libgccjit.h>
#include <stddef.h>

#include "test_assert.h"

static void f1(void* ctx)
{
  int* ptr = (int*)ctx;
  *ptr = 42;
}

static void f2(void* ctx)
{
  int* ptr = (int*)ctx;
  *ptr *= 2;
}

int main(int argc, char const* argv[])
{
  (void)argc;
  (void)argv;

  jit_blocks_funccalls_func_ptr_t records[2] = {&f1, &f2};
  gcc_jit_result* result = NULL;
  jit_blocks_funccalls_output_func_t output_func =
      jit_blocks_funccalls_build(records, 2, &result);
  JB_TEST_ASSERT(output_func != NULL);
  int ctx = 0;
  output_func(&ctx);
  JB_TEST_ASSERT(ctx == 84);
  (void)output_func;
  gcc_jit_result_release(result);
  return 0;
}