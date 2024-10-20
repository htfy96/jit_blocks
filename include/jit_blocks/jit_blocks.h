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

#ifdef __cplusplus
}  // extern "C"
#endif
