#pragma once
#include <sys/types.h>
// ---
#include <libgccjit.h>
#include <stdbool.h>

#include "jit_blocks/jit_blocks_export.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup context_builder Context Builder utilities for easy APIs
    @{
 */

/// A @a jit_blocks_context_builder_t allocates a fresh @a gcc_jit_context,
/// potentially sets some default options, and returns a new context builder.
typedef gcc_jit_context* (*jit_blocks_context_builder_t)(void);
/// Returns the global @a jit_blocks_context_builder_t used for all easy (aka
/// non-aux) APIs. Multithread-unsafe
JIT_BLOCKS_EXPORT jit_blocks_context_builder_t
jit_blocks_get_easy_context_builder(void);

/// Sets the global @a jit_blocks_context_builder_t used for all easy (aka
/// non-aux) APIs. Multithread-unsafe
JIT_BLOCKS_EXPORT void jit_blocks_set_easy_context_builder(
    jit_blocks_context_builder_t builder);

/** @} */
/**
    @defgroup divide divide-by API builders

    Builders to speed up divide-by-constant operations.
    @{
 */
typedef int (*jit_blocks_divide_by_k_int_t)(int dividend);
typedef long (*jit_blocks_divide_by_k_long_t)(long dividend);

JIT_BLOCKS_EXPORT jit_blocks_divide_by_k_int_t
jit_blocks_build_divide_by_k_int(int divisor, gcc_jit_result** out_res);

JIT_BLOCKS_EXPORT jit_blocks_divide_by_k_int_t
jit_blocks_build_divide_by_k_int_aux(int divisor,
                                     gcc_jit_context* custom_context,
                                     gcc_jit_result** out_res);

JIT_BLOCKS_EXPORT jit_blocks_divide_by_k_long_t
jit_blocks_build_divide_by_k_long(long divisor, gcc_jit_result** out_res);

JIT_BLOCKS_EXPORT jit_blocks_divide_by_k_long_t
jit_blocks_build_divide_by_k_long_aux(long divisor,
                                      gcc_jit_context* custom_context,
                                      gcc_jit_result** out_res);
/** @} */

/** @defgroup funccalls Build a series of function calls

    APIs for building a simple function that calls call specified functions
    in order.

    Called functions should accept a single argument of (void*) and return void.

    Given the following constructor call:

    @code{.c}
    jit_blocks_funccalls_func_ptr_t records[2];
    record[0] = &func1;
    record[1] = &func2;
    jit_blocks_funccalls_build(records, 2);
    @endcode

    It will build a function like below:
    @code{.c}
    void output_func(void* arg) {
      func1(arg);
      func2(arg);
      // ...
    }
    @endcode
    @{
 */

typedef void (*jit_blocks_funccalls_func_ptr_t)(void*);

typedef void (*jit_blocks_funccalls_output_func_t)(void* arg);

JIT_BLOCKS_EXPORT jit_blocks_funccalls_output_func_t
jit_blocks_funccalls_build(jit_blocks_funccalls_func_ptr_t* records,
                           int num_records,
                           gcc_jit_result** out_res);

JIT_BLOCKS_EXPORT jit_blocks_funccalls_output_func_t
jit_blocks_funccalls_build_aux(jit_blocks_funccalls_func_ptr_t* records,
                               int num_records,
                               gcc_jit_context* custom_context,
                               gcc_jit_result** out_res);

/** @} */

/** @defgroup expr Efficient floating point arithmetic expression engine

Creates a stack-based expression evaluation function.

Users could pass an array of @ref jit_blocks_expr_func_t to
@ref jit_blocks_expr_build to create a function that accepts a
@ref jit_blocks_expr_context (which is essentially a stack of doubles),

Then, users could call @ref jit_blocks_expr_context_new to create a context,
push doubles into the context via @ref jit_blocks_expr_context_push (Operands
evaluated first should be pushed last so that they could popped out during
calculation first). After users build a successful @ref jit_blocks_expr_context,
they can call the generation function and extract result from the stack via
@ref jit_blocks_expr_context_pop.

Example usage:

@code{.c}
// stack_top *= 22.5
void custom_op(jit_blocks_expr_context* ctx)
{
  double v = 0.;
  jit_blocks_expr_context_pop(ctx, &v);
  jit_blocks_expr_context_push(ctx, v * 22.5);
}
// -((20 + 10) * 5 / 2 - 3)
  jit_blocks_expr_context* ctx = jit_blocks_expr_context_new(32);
  // Note that operands are pushed in reverse order:
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
  assert(out != NULL);
  out(ctx);
  assert(ctx->stack_size == 1);
  assert(fabs(ctx->stack[0] - -1620.) < 1e-5);
@endcode

`jit_blocks_expr_op_*` functions are predefined operators and got specially
handled for faster calculation. See @ref expr-predefined-ops for more details.
@{
*/

/// Context of calculation. Under the hood it's a fixed capacity stack for
/// doubles. Users should use @ref jit_blocks_expr_context_new to create one.
typedef struct jit_blocks_expr_context {
  double* stack;
  int stack_size;
  int stack_capacity;
} jit_blocks_expr_context;

/// Creates a new context with the given initial stack capacity.
JIT_BLOCKS_EXPORT jit_blocks_expr_context* jit_blocks_expr_context_new(
    int initial_stack_capacity);

/// Destroys the context and frees all associated resources.
JIT_BLOCKS_EXPORT void jit_blocks_expr_context_release(
    jit_blocks_expr_context* ctx);

/// Pushes a double onto the stack.
/// @return true if the push operation was successful, false otherwise.
JIT_BLOCKS_EXPORT bool jit_blocks_expr_context_push(
    jit_blocks_expr_context* ctx, double value);

/// Pops a double from the stack.
/// @return true if the pop operation was successful, false otherwise.
JIT_BLOCKS_EXPORT bool jit_blocks_expr_context_pop(jit_blocks_expr_context* ctx,
                                                   double* out_value);

/// Users could pass arbitrary functions to process the expression.
typedef void (*jit_blocks_expr_func_t)(jit_blocks_expr_context* ctx);

/// @defgroup expr-predefined-ops Predefined arithmetic operations
/// \ingroup expr
///
/// expr engine has special support for these operations by inlining,
/// leading to much faster execution.
///

/** @{ */

/// Pops two doubles from the stack, adds them, and pushes the result back.
JIT_BLOCKS_EXPORT void jit_blocks_expr_op_plus(jit_blocks_expr_context* ctx);
/// Pops two doubles from the stack, subtracts the second from the first, and
/// pushes the result back.
JIT_BLOCKS_EXPORT void jit_blocks_expr_op_minus(jit_blocks_expr_context* ctx);
/// Pops two doubles from the stack, multiplies them, and pushes the result
/// back.
JIT_BLOCKS_EXPORT void jit_blocks_expr_op_multiply(
    jit_blocks_expr_context* ctx);
/// Pops two doubles from the stack, divides the first by the second, and
/// pushes the result back. If the divisor is zero, the function will push
/// std::nan()
JIT_BLOCKS_EXPORT void jit_blocks_expr_op_divide(jit_blocks_expr_context* ctx);

/// Pops a double from the stack, negates it, and pushes the result back.
JIT_BLOCKS_EXPORT void jit_blocks_expr_op_negate(jit_blocks_expr_context* ctx);

/// Pops 1 double from the stack, converts it to its absolute value, and pushes
/// the result back.
JIT_BLOCKS_EXPORT void jit_blocks_expr_op_abs(jit_blocks_expr_context* ctx);

/// Pops 2 doubles from the stack, computes the power of the first number to the
/// second number, and pushes the result back. If the first number is zero,
/// and the second number is negative, the function will push nan()
JIT_BLOCKS_EXPORT void jit_blocks_expr_pow(jit_blocks_expr_context* ctx);

/// Parses arithmetic operations from the given string, and stores the parsed
/// operations into @a out_funcs array.
///
/// @param ops Arithmetic expression to parse:
///        - "+" for @ref jit_blocks_expr_op_plus
///        - "-" for @ref jit_blocks_expr_op_minus
///        - "*" for @ref jit_blocks_expr_op_multiply
///        - "/" for @ref jit_blocks_expr_op_divide
///        - "~" for @ref jit_blocks_expr_op_negate
///        - "abs" for @ref jit_blocks_expr_op_abs
///        - "^" for @ref jit_blocks_expr_pow
/// @param ops_cnt Number of operations in @a ops array.
/// @param out_funcs Array to store parsed operations.
JIT_BLOCKS_EXPORT void jit_blocks_expr_ops_parse(
    const char* ops[], int ops_cnt, jit_blocks_expr_func_t* out_funcs);
/// @}

/// Creates a function that performs the given arithmetic operations on the
/// @a initial_ctx stack, and pushes the result back onto the stack.
///

///
/// @returns the built function. NULL if the function creation failed.
JIT_BLOCKS_EXPORT jit_blocks_expr_func_t jit_blocks_expr_build(
    jit_blocks_expr_func_t* ops, int num_ops, gcc_jit_result** out_res);

/// @see jit_blocks_expr_build
JIT_BLOCKS_EXPORT jit_blocks_expr_func_t
jit_blocks_expr_build_aux(jit_blocks_expr_func_t* ops,
                          int num_ops,
                          gcc_jit_context* ctx,
                          gcc_jit_result** out_res);

/// @}

#ifdef __cplusplus
}  // extern "C"
#endif
