#include "jit_blocks/jit_blocks.h"

#include <assert.h>
#include <stddef.h>

#include "libgccjit.h"

int main(int argc, char const* argv[])
{
  (void)argc;
  (void)argv;

  gcc_jit_result* result = NULL;
  jit_blocks_divide_by_k_int_t divide_by_42 =
      jit_blocks_build_divide_by_k_int(42, &result);
  assert(divide_by_42 != NULL);
  assert(divide_by_42(100) == 2);
  assert(divide_by_42(42) == 1);
  assert(divide_by_42(0) == 0);
  assert(divide_by_42(4493) == 106);
  (void)divide_by_42;
  gcc_jit_result_release(result);

  jit_blocks_divide_by_k_long_t devide_by_9285918375 =
      jit_blocks_build_divide_by_k_long(9285918375, &result);
  assert(devide_by_9285918375 != NULL);
  assert(devide_by_9285918375(10000000000) == 1);
  assert(devide_by_9285918375(2132343254325) == 229);
  (void)devide_by_9285918375;
  gcc_jit_result_release(result);
  return 0;
}