#include <assert.h>
// NOLINTNEXTLINE(misc-include-cleaner)
#include <unistd.h>
// --
#include <libgccjit.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "jit_blocks/jit_blocks.h"

jit_blocks_dynswitch_result_t jit_blocks_dynswitch_build(
    const jit_blocks_dynswitch_cond_t* conds,
    int num_conds,
    jit_blocks_dynswitch_func_ptr_t default_func,
    gcc_jit_result** out_res)
{
  gcc_jit_context* ctx = context_builder();
  jit_blocks_dynswitch_result_t result = jit_blocks_dynswitch_build_aux(
      conds, num_conds, default_func, ctx, out_res);
  gcc_jit_context_release(ctx);
  return result;
}

jit_blocks_dynswitch_result_t jit_blocks_dynswitch_build_aux(
    const jit_blocks_dynswitch_cond_t* conds,
    int num_conds,
    jit_blocks_dynswitch_func_ptr_t default_func,
    gcc_jit_context* ctx,
    gcc_jit_result** out_res)
{
  static const char FUNC_NAME[] = "dynswitch_func";
  gcc_jit_type* void_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_VOID);
  gcc_jit_type* void_ptr_type =
      gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_VOID_PTR);
  gcc_jit_type* long_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_LONG);
  gcc_jit_type* func_ptr_param_types[] = {void_ptr_type, long_type};
  gcc_jit_type* func_ptr_type = gcc_jit_context_new_function_ptr_type(
      ctx, NULL, void_type, 2, func_ptr_param_types, 0);

  gcc_jit_param* params[] = {
      gcc_jit_context_new_param(ctx, NULL, void_ptr_type, "ctx"),
      gcc_jit_context_new_param(ctx, NULL, long_type, "val")};
  gcc_jit_function* func = gcc_jit_context_new_function(
      ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, void_type, FUNC_NAME, 2, params, 0);
  gcc_jit_rvalue* arg_ctx =
      gcc_jit_param_as_rvalue(gcc_jit_function_get_param(func, 0));
  gcc_jit_rvalue* arg_val =
      gcc_jit_param_as_rvalue(gcc_jit_function_get_param(func, 1));
  gcc_jit_block* prev_block = gcc_jit_function_new_block(func, "entry_block");
  gcc_jit_block* default_block =
      gcc_jit_function_new_block(func, "default_block");
  gcc_jit_rvalue* call_args[] = {arg_ctx, arg_val};
  for (int i = 0; i < num_conds; ++i) {
    const jit_blocks_dynswitch_cond_t* cond = conds + i;
    gcc_jit_rvalue* cmp_result = gcc_jit_context_new_comparison(
        ctx,
        NULL,
        GCC_JIT_COMPARISON_EQ,
        arg_val,
        gcc_jit_context_new_rvalue_from_long(ctx, long_type, cond->val));
    char block_name[20];
    JB_ASSERT(snprintf(block_name, sizeof(block_name), "case_%ld", cond->val)
              >= 0);
    gcc_jit_block* success_blk = gcc_jit_function_new_block(func, block_name);

    gcc_jit_block_add_eval(
        success_blk,
        NULL,
        gcc_jit_context_new_call_through_ptr(
            ctx,
            NULL,
            gcc_jit_context_new_rvalue_from_ptr(ctx, func_ptr_type, cond->func),
            2,
            call_args));
    gcc_jit_block_end_with_void_return(success_blk, NULL);
    gcc_jit_block* nxt_block = default_block;
    if (i < num_conds - 1) {
      nxt_block = gcc_jit_function_new_block(func, NULL);
    }

    gcc_jit_block_end_with_conditional(
        prev_block, NULL, cmp_result, success_blk, nxt_block);
    prev_block = nxt_block;
  }
  gcc_jit_block_add_eval(
      default_block,
      NULL,
      gcc_jit_context_new_call_through_ptr(
          ctx,
          NULL,
          gcc_jit_context_new_rvalue_from_ptr(ctx, func_ptr_type, default_func),
          2,
          call_args));
  gcc_jit_block_end_with_void_return(default_block, NULL);

  *out_res = gcc_jit_context_compile(ctx);
  return (jit_blocks_dynswitch_result_t)gcc_jit_result_get_code(*out_res,
                                                                FUNC_NAME);
}