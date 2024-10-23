
#include <assert.h>
// NOLINTNEXTLINE(misc-include-cleaner)
#include <unistd.h>
// --
#include <libgccjit.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "jit_blocks/jit_blocks.h"

jit_blocks_divide_by_k_int_t jit_blocks_build_divide_by_k_int(
    int divisor, gcc_jit_result** out_res)
{
  gcc_jit_context* ctx = context_builder();
  jit_blocks_divide_by_k_int_t result =
      jit_blocks_build_divide_by_k_int_aux(divisor, ctx, out_res);
  gcc_jit_context_release(ctx);
  return result;
}

#define MAX_FUNC_NAME_BUFFER_SIZE 40

jit_blocks_divide_by_k_int_t jit_blocks_build_divide_by_k_int_aux(
    int divisor, gcc_jit_context* custom_context, gcc_jit_result** out_res)
{
  gcc_jit_type* int_type =
      gcc_jit_context_get_type(custom_context, GCC_JIT_TYPE_INT);
  gcc_jit_param* param_i =
      gcc_jit_context_new_param(custom_context, NULL, int_type, "dividend");
  char func_name_buffer[MAX_FUNC_NAME_BUFFER_SIZE] = {};
  int ret = snprintf(
      func_name_buffer, sizeof(func_name_buffer), "divide_by_%d", divisor);
  (void)ret;
  assert(ret < (int)sizeof(func_name_buffer));
  gcc_jit_function* func =
      gcc_jit_context_new_function(custom_context,
                                   NULL,
                                   GCC_JIT_FUNCTION_EXPORTED,
                                   int_type,
                                   func_name_buffer,
                                   1,
                                   &param_i,
                                   0);

  gcc_jit_block* block = gcc_jit_function_new_block(func, NULL);
  gcc_jit_rvalue* expr = gcc_jit_context_new_binary_op(
      custom_context,
      NULL,
      GCC_JIT_BINARY_OP_DIVIDE,
      int_type,
      gcc_jit_param_as_rvalue(param_i),
      gcc_jit_context_new_rvalue_from_int(custom_context, int_type, divisor));
  gcc_jit_block_end_with_return(block, NULL, expr);
  *out_res = gcc_jit_context_compile(custom_context);
  return (jit_blocks_divide_by_k_int_t)gcc_jit_result_get_code(
      *out_res, func_name_buffer);
}

jit_blocks_divide_by_k_long_t jit_blocks_build_divide_by_k_long(
    long divisor, gcc_jit_result** out_res)
{
  gcc_jit_context* ctx = context_builder();
  jit_blocks_divide_by_k_long_t result =
      jit_blocks_build_divide_by_k_long_aux(divisor, ctx, out_res);
  gcc_jit_context_release(ctx);
  return result;
}

jit_blocks_divide_by_k_long_t jit_blocks_build_divide_by_k_long_aux(
    long divisor, gcc_jit_context* custom_context, gcc_jit_result** out_res)
{
  gcc_jit_type* long_type =
      gcc_jit_context_get_type(custom_context, GCC_JIT_TYPE_LONG);
  gcc_jit_param* param_i =
      gcc_jit_context_new_param(custom_context, NULL, long_type, "dividend");
  char func_name_buffer[MAX_FUNC_NAME_BUFFER_SIZE] = {};
  int ret = snprintf(
      func_name_buffer, sizeof(func_name_buffer), "divide_by_%ld", divisor);
  (void)ret;
  assert(ret < (int)sizeof(func_name_buffer));
  gcc_jit_function* func =
      gcc_jit_context_new_function(custom_context,
                                   NULL,
                                   GCC_JIT_FUNCTION_EXPORTED,
                                   long_type,
                                   func_name_buffer,
                                   1,
                                   &param_i,
                                   0);

  gcc_jit_block* block = gcc_jit_function_new_block(func, NULL);
  gcc_jit_rvalue* expr = gcc_jit_context_new_binary_op(
      custom_context,
      NULL,
      GCC_JIT_BINARY_OP_DIVIDE,
      long_type,
      gcc_jit_param_as_rvalue(param_i),
      gcc_jit_context_new_rvalue_from_long(custom_context, long_type, divisor));
  gcc_jit_block_end_with_return(block, NULL, expr);
  *out_res = gcc_jit_context_compile(custom_context);
  return (jit_blocks_divide_by_k_long_t)gcc_jit_result_get_code(
      *out_res, func_name_buffer);
}
