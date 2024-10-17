#include "jit_blocks/jit_blocks.h"

#include <string.h>

int main(int argc, char const* argv[])
{
  (void)argc;
  (void)argv;

  return strcmp(exported_function(), "jit_blocks") == 0 ? 0 : 1;
}
