#include <assert.h>
#include <time.h>
// NOLINTNEXTLINE(misc-include-cleaner)
#include <unistd.h>
// --
#include <libgccjit.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "jit_blocks/jit_blocks.h"

jit_blocks_expr_context* jit_blocks_expr_context_new(int initial_stack_capacity)
{
  jit_blocks_expr_context* result = malloc(sizeof(jit_blocks_expr_context));
  result->stack = malloc((size_t)initial_stack_capacity * sizeof(double));
  result->stack_size = 0;
  result->stack_capacity = initial_stack_capacity;
  return result;
}
void jit_blocks_expr_context_release(jit_blocks_expr_context* ctx)
{
  if (ctx) {
    free(ctx->stack);
  }
  free(ctx);
}

bool jit_blocks_expr_context_push(jit_blocks_expr_context* ctx, double value)
{
  if (ctx->stack_size >= ctx->stack_capacity) {
    return false;
  }
  ctx->stack[ctx->stack_size++] = value;
  return true;
}

bool jit_blocks_expr_context_pop(jit_blocks_expr_context* ctx,
                                 double* out_value)
{
  if (ctx->stack_size == 0) {
    return false;
  }
  *out_value = ctx->stack[--ctx->stack_size];
  return true;
}

void jit_blocks_expr_op_plus(jit_blocks_expr_context* ctx)
{
  double right;
  double left;
  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &right));
  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &left));
  JB_ASSERT(jit_blocks_expr_context_push(ctx, left + right));
}

void jit_blocks_expr_op_minus(jit_blocks_expr_context* ctx)
{
  double right;
  double left;

  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &right));
  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &left));
  JB_ASSERT(jit_blocks_expr_context_push(ctx, left - right));
}

void jit_blocks_expr_op_multiply(jit_blocks_expr_context* ctx)
{
  double right;
  double left;

  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &right));
  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &left));
  JB_ASSERT(jit_blocks_expr_context_push(ctx, left * right));
}

void jit_blocks_expr_op_divide(jit_blocks_expr_context* ctx)
{
  double right;
  double left;

  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &right));
  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &left));

  if (right == 0.0) {
    JB_ASSERT(jit_blocks_expr_context_push(ctx, nan("")));
  } else {
    JB_ASSERT(jit_blocks_expr_context_push(ctx, left / right));
  }
}

void jit_blocks_expr_op_negate(jit_blocks_expr_context* ctx)
{
  double value;
  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &value));
  JB_ASSERT(jit_blocks_expr_context_push(ctx, -value));
}

void jit_blocks_expr_op_abs(jit_blocks_expr_context* ctx)
{
  double value;
  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &value));
  JB_ASSERT(jit_blocks_expr_context_push(ctx, fabs(value)));
}

void jit_blocks_expr_op_power(jit_blocks_expr_context* ctx)
{
  double exponent;
  double base;
  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &exponent));
  JB_ASSERT(jit_blocks_expr_context_pop(ctx, &base));

  if (base == 0.0 && exponent < 0.0) {
    JB_ASSERT(jit_blocks_expr_context_push(ctx, nan("")));
  } else {
    JB_ASSERT(jit_blocks_expr_context_push(ctx, pow(base, exponent)));
  }
}

void jit_blocks_expr_ops_parse(const char* ops[],
                               int ops_cnt,
                               jit_blocks_expr_func_t* out_funcs)
{
  for (int i = 0; i < ops_cnt; ++i) {
    if (strcmp(ops[i], "-") == 0) {
      out_funcs[i] = jit_blocks_expr_op_minus;
    } else if (strcmp(ops[i], "*") == 0) {
      out_funcs[i] = jit_blocks_expr_op_multiply;
    } else if (strcmp(ops[i], "/") == 0) {
      out_funcs[i] = jit_blocks_expr_op_divide;
    } else if (strcmp(ops[i], "~") == 0) {
      out_funcs[i] = jit_blocks_expr_op_negate;
    } else if (strcmp(ops[i], "abs") == 0) {
      out_funcs[i] = jit_blocks_expr_op_abs;
    } else if (strcmp(ops[i], "^") == 0) {
      out_funcs[i] = jit_blocks_expr_op_power;
    }
  }
}

jit_blocks_expr_func_t jit_blocks_expr_build(const jit_blocks_expr_func_t* ops,
                                             int num_ops,
                                             gcc_jit_result** out_res)
{
  gcc_jit_context* ctx = context_builder();
  jit_blocks_expr_func_t result =
      jit_blocks_expr_build_aux(ops, num_ops, ctx, out_res);
  gcc_jit_context_release(ctx);
  return result;
}

jit_blocks_expr_func_t jit_blocks_expr_build_aux(
    const jit_blocks_expr_func_t* ops,
    int num_ops,
    gcc_jit_context* ctx,
    gcc_jit_result** out_res)
{
  static const char* FUNC_NAME = "expr";
  gcc_jit_type* double_type =
      gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_DOUBLE);
  gcc_jit_type* double_ptr_type = gcc_jit_type_get_pointer(double_type);
  gcc_jit_type* int_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_INT);
  gcc_jit_type* void_type = gcc_jit_context_get_type(ctx, GCC_JIT_TYPE_VOID);

  gcc_jit_field* field_stack =
      gcc_jit_context_new_field(ctx, NULL, double_ptr_type, "stack");
  gcc_jit_field* field_stack_size =
      gcc_jit_context_new_field(ctx, NULL, int_type, "stack_size");
  gcc_jit_field* field_stack_capacity =
      gcc_jit_context_new_field(ctx, NULL, int_type, "stack_capacity");

  gcc_jit_field* expr_context_fields[] = {
      field_stack,
      field_stack_size,
      field_stack_capacity,
  };
  gcc_jit_type* stack_ctx_type =
      gcc_jit_struct_as_type(gcc_jit_context_new_struct_type(
          ctx, NULL, "jit_blocks_expr_context", 3, expr_context_fields));

  gcc_jit_type* stack_ctx_ptr_type = gcc_jit_type_get_pointer(stack_ctx_type);
  gcc_jit_type* func_ptr_type = gcc_jit_context_new_function_ptr_type(
      ctx, NULL, void_type, 1, &stack_ctx_ptr_type, 0);

  gcc_jit_param* param =
      gcc_jit_context_new_param(ctx, NULL, stack_ctx_ptr_type, "arg");
  gcc_jit_function* func = gcc_jit_context_new_function(
      ctx, NULL, GCC_JIT_FUNCTION_EXPORTED, void_type, FUNC_NAME, 1, &param, 0);
  gcc_jit_block* entry_block = gcc_jit_function_new_block(func, NULL);

  gcc_jit_rvalue* ctx_param =
      gcc_jit_param_as_rvalue(gcc_jit_function_get_param(func, 0));
  gcc_jit_lvalue* stack =
      gcc_jit_rvalue_dereference_field(ctx_param, NULL, field_stack);
  gcc_jit_lvalue* stack_size =
      gcc_jit_rvalue_dereference_field(ctx_param, NULL, field_stack_size);
  gcc_jit_lvalue* stack_capacity =
      gcc_jit_rvalue_dereference_field(ctx_param, NULL, field_stack_capacity);
  (void)stack_capacity;
  for (int i = 0; i < num_ops; ++i) {
    jit_blocks_expr_func_t op = ops[i];
    gcc_jit_rvalue* stack_top_index =
        gcc_jit_context_new_binary_op(ctx,
                                      NULL,
                                      GCC_JIT_BINARY_OP_MINUS,
                                      int_type,
                                      gcc_jit_lvalue_as_rvalue(stack_size),
                                      gcc_jit_context_one(ctx, int_type));
    gcc_jit_rvalue* stack_top2_index = gcc_jit_context_new_binary_op(
        ctx,
        NULL,
        GCC_JIT_BINARY_OP_MINUS,
        int_type,
        gcc_jit_lvalue_as_rvalue(stack_size),
        gcc_jit_context_new_rvalue_from_int(ctx, int_type, 2));

    gcc_jit_rvalue* a =
        gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(
            ctx, NULL, gcc_jit_lvalue_as_rvalue(stack), stack_top_index));

    gcc_jit_rvalue* b =
        gcc_jit_lvalue_as_rvalue(gcc_jit_context_new_array_access(
            ctx, NULL, gcc_jit_lvalue_as_rvalue(stack), stack_top2_index));
    if (op == jit_blocks_expr_op_plus) {
      gcc_jit_rvalue* res = gcc_jit_context_new_binary_op(
          ctx, NULL, GCC_JIT_BINARY_OP_PLUS, double_type, a, b);
      gcc_jit_block_add_assignment(
          entry_block,
          NULL,
          gcc_jit_context_new_array_access(
              ctx, NULL, gcc_jit_lvalue_as_rvalue(stack), stack_top2_index),
          res);
      gcc_jit_block_add_assignment(
          entry_block,
          NULL,
          stack_size,
          gcc_jit_context_new_binary_op(ctx,
                                        NULL,
                                        GCC_JIT_BINARY_OP_MINUS,
                                        int_type,
                                        gcc_jit_lvalue_as_rvalue(stack_size),
                                        gcc_jit_context_one(ctx, int_type)));
    } else if (op == jit_blocks_expr_op_minus) {
      gcc_jit_rvalue* res = gcc_jit_context_new_binary_op(
          ctx, NULL, GCC_JIT_BINARY_OP_MINUS, double_type, a, b);
      gcc_jit_block_add_assignment(
          entry_block,
          NULL,
          gcc_jit_context_new_array_access(
              ctx, NULL, gcc_jit_lvalue_as_rvalue(stack), stack_top2_index),
          res);
      gcc_jit_block_add_assignment(
          entry_block,
          NULL,
          stack_size,
          gcc_jit_context_new_binary_op(ctx,
                                        NULL,
                                        GCC_JIT_BINARY_OP_MINUS,
                                        int_type,
                                        gcc_jit_lvalue_as_rvalue(stack_size),
                                        gcc_jit_context_one(ctx, int_type)));
    } else if (op == jit_blocks_expr_op_multiply) {
      gcc_jit_rvalue* res = gcc_jit_context_new_binary_op(
          ctx, NULL, GCC_JIT_BINARY_OP_MULT, double_type, a, b);
      gcc_jit_block_add_assignment(
          entry_block,
          NULL,
          gcc_jit_context_new_array_access(
              ctx, NULL, gcc_jit_lvalue_as_rvalue(stack), stack_top2_index),
          res);
      gcc_jit_block_add_assignment(
          entry_block,
          NULL,
          stack_size,
          gcc_jit_context_new_binary_op(ctx,
                                        NULL,
                                        GCC_JIT_BINARY_OP_MINUS,
                                        int_type,
                                        gcc_jit_lvalue_as_rvalue(stack_size),
                                        gcc_jit_context_one(ctx, int_type)));
    } else if (op == jit_blocks_expr_op_divide) {
      gcc_jit_rvalue* res = gcc_jit_context_new_binary_op(
          ctx, NULL, GCC_JIT_BINARY_OP_DIVIDE, double_type, a, b);
      gcc_jit_block_add_assignment(
          entry_block,
          NULL,
          gcc_jit_context_new_array_access(
              ctx, NULL, gcc_jit_lvalue_as_rvalue(stack), stack_top2_index),
          res);
      gcc_jit_block_add_assignment(
          entry_block,
          NULL,
          stack_size,
          gcc_jit_context_new_binary_op(ctx,
                                        NULL,
                                        GCC_JIT_BINARY_OP_MINUS,
                                        int_type,
                                        gcc_jit_lvalue_as_rvalue(stack_size),
                                        gcc_jit_context_one(ctx, int_type)));
    } else if (op == jit_blocks_expr_op_negate) {
      gcc_jit_rvalue* res = gcc_jit_context_new_unary_op(
          ctx, NULL, GCC_JIT_UNARY_OP_MINUS, double_type, a);
      gcc_jit_block_add_assignment(
          entry_block,
          NULL,
          gcc_jit_context_new_array_access(
              ctx, NULL, gcc_jit_lvalue_as_rvalue(stack), stack_top_index),
          res);
    } else if (op == jit_blocks_expr_op_abs) {
      gcc_jit_rvalue* res = gcc_jit_context_new_unary_op(
          ctx, NULL, GCC_JIT_UNARY_OP_ABS, double_type, a);
      gcc_jit_block_add_assignment(
          entry_block,
          NULL,
          gcc_jit_context_new_array_access(
              ctx, NULL, gcc_jit_lvalue_as_rvalue(stack), stack_top_index),
          res);
    } else {
      // fall back to the func call case
      gcc_jit_block_add_eval(
          entry_block,
          NULL,
          gcc_jit_context_new_call_through_ptr(
              ctx,
              NULL,
              gcc_jit_context_new_rvalue_from_ptr(ctx, func_ptr_type, op),
              1,
              &ctx_param));
    }
  }
  gcc_jit_block_end_with_void_return(entry_block, NULL);
  *out_res = gcc_jit_context_compile(ctx);
  if (out_res) {
    return (jit_blocks_expr_func_t)gcc_jit_result_get_code(*out_res, FUNC_NAME);
  }
  return NULL;
}