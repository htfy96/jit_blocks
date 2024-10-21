cmake_minimum_required(VERSION 3.21)

foreach(var IN ITEMS PROJECT_BINARY_DIR PROJECT_SOURCE_DIR)
  if(NOT DEFINED "${var}")
    message(FATAL_ERROR "${var} must be defined")
  endif()
endforeach()
set(bin "${PROJECT_BINARY_DIR}")
set(src "${PROJECT_SOURCE_DIR}")

# ---- Dependencies ----

set(doxygen_awesome_SOURCE_DIR "${bin}/docs/.ci")
if(NOT IS_DIRECTORY "${doxygen_awesome_SOURCE_DIR}")
  file(MAKE_DIRECTORY "${doxygen_awesome_SOURCE_DIR}")
  file(
      DOWNLOAD
      https://github.com/jothepro/doxygen-awesome-css/archive/refs/heads/main.zip
      "${doxygen_awesome_SOURCE_DIR}/doxygen_awesome.zip"
      STATUS status
  )
  if(NOT status MATCHES "^0;")
    message(FATAL_ERROR "Download failed with ${status}")
  endif()
  execute_process(
      COMMAND "${CMAKE_COMMAND}" -E tar xf doxygen_awesome.zip
      WORKING_DIRECTORY "${doxygen_awesome_SOURCE_DIR}"
      RESULT_VARIABLE result
  )
  if(NOT result EQUAL "0")
    message(FATAL_ERROR "Extraction failed with ${result}")
  endif()
  file(REMOVE "${doxygen_awesome_SOURCE_DIR}/doxygen_awesome.zip")
endif()

find_program(Python3_EXECUTABLE NAMES python3 python)
if(NOT Python3_EXECUTABLE)
  message(FATAL_ERROR "Python executable was not found")
endif()

# ---- Process project() call in CMakeLists.txt ----

file(READ "${src}/CMakeLists.txt" content)

string(FIND "${content}" "project(" index)
if(index EQUAL "-1")
  message(FATAL_ERROR "Could not find \"project(\"")
endif()
string(SUBSTRING "${content}" "${index}" -1 content)

string(FIND "${content}" "\n)\n" index)
if(index EQUAL "-1")
  message(FATAL_ERROR "Could not find \"\\n)\\n\"")
endif()
string(SUBSTRING "${content}" 0 "${index}" content)

file(WRITE "${bin}/docs-ci.project.cmake" "docs_${content}\n)\n")

macro(list_pop_front list out)
  list(GET "${list}" 0 "${out}")
  list(REMOVE_AT "${list}" 0)
endmacro()

function(docs_project name)
  cmake_parse_arguments(PARSE_ARGV 1 "" "" "VERSION;DESCRIPTION;HOMEPAGE_URL" LANGUAGES)
  set(PROJECT_NAME "${name}" PARENT_SCOPE)
  if(DEFINED _VERSION)
    set(PROJECT_VERSION "${_VERSION}" PARENT_SCOPE)
    string(REGEX MATCH "^[0-9]+(\\.[0-9]+)*" versions "${_VERSION}")
    string(REPLACE . ";" versions "${versions}")
    set(suffixes MAJOR MINOR PATCH TWEAK)
    while(NOT versions STREQUAL "" AND NOT suffixes STREQUAL "")
      list_pop_front(versions version)
      list_pop_front(suffixes suffix)
      set("PROJECT_VERSION_${suffix}" "${version}" PARENT_SCOPE)
    endwhile()
  endif()
  if(DEFINED _DESCRIPTION)
    set(PROJECT_DESCRIPTION "${_DESCRIPTION}" PARENT_SCOPE)
  endif()
  if(DEFINED _HOMEPAGE_URL)
    set(PROJECT_HOMEPAGE_URL "${_HOMEPAGE_URL}" PARENT_SCOPE)
  endif()
endfunction()

include("${bin}/docs-ci.project.cmake")

# ---- Generate docs ----

if(NOT DEFINED DOXYGEN_OUTPUT_DIRECTORY)
  set(DOXYGEN_OUTPUT_DIRECTORY "${bin}/docs")
endif()
set(out "${DOXYGEN_OUTPUT_DIRECTORY}")

set(SOURCE_DOC_CONFIG_DIR "${PROJECT_SOURCE_DIR}/docs")

set(AWESOME_CSS_DIR ${doxygen_awesome_SOURCE_DIR})

foreach(file IN ITEMS Doxyfile conf.py)
  configure_file("${src}/docs/${file}.in" "${bin}/docs/${file}" @ONLY)
endforeach()

set(doxygen_awesome_script "${doxygen_awesome_SOURCE_DIR}/documentation/doxygen.py")
set(config "${bin}/docs/Doxyfile")

file(REMOVE_RECURSE "${out}/html" "${out}/xml")

execute_process(
    COMMAND "doxygen" "${config}"
    WORKING_DIRECTORY "${bin}/docs"
    RESULT_VARIABLE result
)
if(NOT result EQUAL "0")
  message(FATAL_ERROR "m.css returned with ${result}")
endif()
