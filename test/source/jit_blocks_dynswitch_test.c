#include "jit_blocks/jit_blocks.h"

#include <stdlib.h>

#include "libgccjit.h"
#include "test_assert.h"

long last_called = 0;

void f1(void* ctx, long val)
{
  *(int*)ctx *= 2;
  last_called = val;
}

void f2(void* ctx, long val)
{
  *(int*)ctx *= 3;
  last_called = val * 2;
}

void f3(void* ctx, long val)
{
  *(int*)ctx *= 4;
  last_called = val * 3;
}

int main(void)
{
  int* ctx = malloc(sizeof(int));
  *ctx = 1;
  jit_blocks_dynswitch_cond_t switch_conds[] = {
      {.func = f1, .val = 42},
      {.func = f2, .val = -15},
  };
  gcc_jit_result* res = NULL;
  jit_blocks_dynswitch_result_t result =
      jit_blocks_dynswitch_build(switch_conds, 2, f3, &res);
  JB_TEST_ASSERT(res != NULL);
  JB_TEST_ASSERT(result != NULL);
  result(ctx, 10);
  JB_TEST_ASSERT(*ctx == 4);
  JB_TEST_ASSERT(last_called == 30);
  result(ctx, 42);
  JB_TEST_ASSERT(*ctx == 8);
  JB_TEST_ASSERT(last_called == 42);
  result(ctx, -15);
  JB_TEST_ASSERT(*ctx == 24);
  JB_TEST_ASSERT(last_called == -30);
  free(ctx);
  gcc_jit_result_release(res);
  return 0;
}