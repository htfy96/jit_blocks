file(GLOB libgccjit_INCLUDE_PATHS "/usr/lib/gcc/x86_64-linux-gnu/*/include/")


find_path(libgccjit_INCLUDE_DIR
    NAMES libgccjit.h
    PATHS ${libgccjit_INCLUDE_PATHS}
)

if (libgccjit_INCLUDE_DIR)
    message(STATUS "Found libgccjit include dir: ${libgccjit_INCLUDE_DIR}")
    find_library(libgccjit_LIBRARY
        NAMES gccjit
        PATHS ${libgccjit_INCLUDE_DIR} ${libgccjit_INCLUDE_DIR}/..
    )
else()
    message(FATAL_ERROR "libgccjit include dir not found")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libgccjit
  FOUND_VAR libgccjit_FOUND
  REQUIRED_VARS
    libgccjit_INCLUDE_DIR
    libgccjit_LIBRARY
)

if(libgccjit_FOUND AND NOT TARGET libgccjit::libgccjit)

  add_library(libgccjit::libgccjit SHARED IMPORTED)
  set_target_properties(libgccjit::libgccjit PROPERTIES
    IMPORTED_LOCATION "${libgccjit_LIBRARY}"
  )
  target_include_directories(libgccjit::libgccjit INTERFACE
    ${libgccjit_INCLUDE_DIR})
endif()

mark_as_advanced(
  libgccjit_INCLUDE_DIR
  libgccjit_LIBRARY
)