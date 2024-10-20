#include "jit_blocks/jit_blocks.h"

#include <assert.h>

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
  jit_blocks_funccalls_output_func_t output_func =
      jit_blocks_funccalls_build(records, 2);
  assert(output_func != NULL);
  int ctx = 0;
  output_func(&ctx);
  assert(ctx == 84);
  (void)output_func;
  return 0;
}