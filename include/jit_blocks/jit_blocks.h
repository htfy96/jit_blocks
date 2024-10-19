#pragma once
#include <sys/types.h>
// ---
#include <libgccjit.h>

#include "jit_blocks/jit_blocks_export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef gcc_jit_context* (*jit_blocks_context_builder_t)(void);
JIT_BLOCKS_EXPORT jit_blocks_context_builder_t
jit_blocks_get_easy_context_builder(void);

JIT_BLOCKS_EXPORT void jit_blocks_set_easy_context_builder(
    jit_blocks_context_builder_t builder);

typedef int (*jit_blocks_divide_by_k_int_t)(int dividend);
typedef long (*jit_blocks_divide_by_k_long_t)(long dividend);

JIT_BLOCKS_EXPORT jit_blocks_divide_by_k_int_t
jit_blocks_build_divide_by_k_int(int divisor);

JIT_BLOCKS_EXPORT jit_blocks_divide_by_k_int_t
jit_blocks_build_divide_by_k_int_aux(int divisor,
                                     gcc_jit_context* custom_context);

JIT_BLOCKS_EXPORT jit_blocks_divide_by_k_long_t
jit_blocks_build_divide_by_k_long(long divisor);

JIT_BLOCKS_EXPORT jit_blocks_divide_by_k_long_t
jit_blocks_build_divide_by_k_long_aux(long divisor,
                                      gcc_jit_context* custom_context);

#ifdef __cplusplus
}  // extern "C"
#endif
