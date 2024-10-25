#include <assert.h>
// NOLINTNEXTLINE(misc-include-cleaner)
#include <unistd.h>
// --
#include <libgccjit.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "jit_blocks/jit_blocks.h"

jit_blocks_funccalls_output_func_t jit_blocks_funccalls_build(
    const jit_blocks_funccalls_func_ptr_t* records,
    int num_records,
    gcc_jit_result** out_res)
{
  gcc_jit_context* context = context_builder();
  jit_blocks_funccalls_output_func_t result =
      jit_blocks_funccalls_build_aux(records, num_records, context, out_res);
  gcc_jit_context_release(context);
  return result;
}

jit_blocks_funccalls_output_func_t jit_blocks_funccalls_build_aux(
    const jit_blocks_funccalls_func_ptr_t* records,
    int num_records,
    gcc_jit_context* custom_context,
    gcc_jit_result** out_res)
{
  if (out_res == NULL) {
    return NULL;
  }
  static const char* FUNC_NAME = "funccall";
  gcc_jit_type* void_ptr_type =
      gcc_jit_context_get_type(custom_context, GCC_JIT_TYPE_VOID_PTR);
  gcc_jit_type* void_type =
      gcc_jit_context_get_type(custom_context, GCC_JIT_TYPE_VOID);

  gcc_jit_param* param_i =
      gcc_jit_context_new_param(custom_context, NULL, void_ptr_type, "arg");

  gcc_jit_function* func =
      gcc_jit_context_new_function(custom_context,
                                   NULL,
                                   GCC_JIT_FUNCTION_EXPORTED,
                                   void_type,
                                   FUNC_NAME,
                                   1,
                                   &param_i,
                                   0);

  if (!func) {
    return NULL;
  }

  gcc_jit_param* arg = gcc_jit_function_get_param(func, 0);
  gcc_jit_type* func_ptr_param_types[1] = {
      void_ptr_type,
  };
  gcc_jit_type* func_ptr_type = gcc_jit_context_new_function_ptr_type(
      custom_context, NULL, void_type, 1, func_ptr_param_types, 0);

  gcc_jit_block* block = gcc_jit_function_new_block(func, NULL);

  for (int i = 0; i < num_records; ++i) {
    const jit_blocks_funccalls_func_ptr_t* record = &records[i];
    gcc_jit_rvalue* call_args[1] = {gcc_jit_param_as_rvalue(arg)};
    gcc_jit_block_add_eval(
        block,
        NULL,
        gcc_jit_context_new_call_through_ptr(
            custom_context,
            NULL,
            gcc_jit_context_new_rvalue_from_ptr(
                custom_context, func_ptr_type, (void*)*record),
            1,
            call_args));
  }

  gcc_jit_block_end_with_void_return(block, NULL);
  *out_res = gcc_jit_context_compile(custom_context);
  if (!*out_res) {
    return NULL;
  }
  return (jit_blocks_funccalls_output_func_t)gcc_jit_result_get_code(*out_res,
                                                                     FUNC_NAME);
}