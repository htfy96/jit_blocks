# ---- Dependencies ----

set(extract_timestamps "")
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
  set(extract_timestamps DOWNLOAD_EXTRACT_TIMESTAMP YES)
endif()

include(FetchContent)
FetchContent_Declare(
    doxygen_awesome 
    GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
    GIT_TAG v2.3.4
    SOURCE_DIR "${PROJECT_BINARY_DIR}/doxygen_awesome"
    UPDATE_DISCONNECTED YES
    ${extract_timestamps}
)
FetchContent_MakeAvailable(doxygen_awesome)

FetchContent_GetProperties(doxygen_awesome SOURCE_DIR AWESOME_CSS_DIR)

find_package(Python3 3.6 REQUIRED)

# ---- Declare documentation target ----

set(
    DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/docs"
    CACHE PATH "Path for the generated Doxygen documentation"
)

set(working_dir "${PROJECT_BINARY_DIR}/docs")
set(SOURCE_DOC_CONFIG_DIR "${PROJECT_SOURCE_DIR}/docs")

foreach(file IN ITEMS Doxyfile conf.py)
  configure_file("docs/${file}.in" "${working_dir}/${file}" @ONLY)
endforeach()

set(config "${working_dir}/Doxyfile")

add_custom_target(
    docs
    COMMAND "${CMAKE_COMMAND}" -E remove_directory
    "${DOXYGEN_OUTPUT_DIRECTORY}/html"
    "${DOXYGEN_OUTPUT_DIRECTORY}/xml"
    COMMAND "doxygen" "${config}"
    COMMENT "Building documentation using Doxygen and m.css"
    WORKING_DIRECTORY "${working_dir}"
    VERBATIM
)
