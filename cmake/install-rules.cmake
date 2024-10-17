if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/jit_blocks-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package jit_blocks)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT jit_blocks_Development
)

install(
    TARGETS jit_blocks_jit_blocks
    EXPORT jit_blocksTargets
    RUNTIME #
    COMPONENT jit_blocks_Runtime
    LIBRARY #
    COMPONENT jit_blocks_Runtime
    NAMELINK_COMPONENT jit_blocks_Development
    ARCHIVE #
    COMPONENT jit_blocks_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    jit_blocks_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE jit_blocks_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(jit_blocks_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${jit_blocks_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT jit_blocks_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${jit_blocks_INSTALL_CMAKEDIR}"
    COMPONENT jit_blocks_Development
)

install(
    EXPORT jit_blocksTargets
    NAMESPACE jit_blocks::
    DESTINATION "${jit_blocks_INSTALL_CMAKEDIR}"
    COMPONENT jit_blocks_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
