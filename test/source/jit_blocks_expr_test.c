#include "jit_blocks/jit_blocks.h"

#include <assert.h>
#include <libgccjit.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

#include "test_assert.h"
// * 22.5
void custom_op(jit_blocks_expr_context* ctx)
{
  double v = 0.;
  bool res = jit_blocks_expr_context_pop(ctx, &v);
  (void)res;
  assert(res);
  jit_blocks_expr_context_push(ctx, v * 22.5);
}

int main(void)
{
  // -((20 + 10) * 5 / 2 - 3)
  jit_blocks_expr_context* ctx = jit_blocks_expr_context_new(32);

  jit_blocks_expr_context_push(ctx, 3);
  jit_blocks_expr_context_push(ctx, 2);
  jit_blocks_expr_context_push(ctx, 5);
  jit_blocks_expr_context_push(ctx, 10);
  jit_blocks_expr_context_push(ctx, 20);
  jit_blocks_expr_func_t funcs[] = {&jit_blocks_expr_op_plus,
                                    &jit_blocks_expr_op_multiply,
                                    &jit_blocks_expr_op_divide,
                                    &jit_blocks_expr_op_minus,
                                    &jit_blocks_expr_op_negate,
                                    &custom_op};
  gcc_jit_result* result = NULL;
  jit_blocks_expr_func_t out = jit_blocks_expr_build(funcs, 6, &result);
  JB_TEST_ASSERT(out != NULL);
  out(ctx);
  JB_TEST_ASSERT(ctx->stack_size == 1);
  JB_TEST_ASSERT(fabs(ctx->stack[0] - -1620.) < 1e-5);
  gcc_jit_result_release(result);
  printf("Pass!\n");
  return 0;
}