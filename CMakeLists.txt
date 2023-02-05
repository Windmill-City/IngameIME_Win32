cmake_minimum_required(VERSION 3.18)
project(IngameIME-Win32)

# ##############################################################################
# Compiler flags
# ##############################################################################
set(CMAKE_CXX_STANDARD 17)

if(MSVC)
  # Use Static Runtime Library
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ##############################################################################
# Find Packages
# ##############################################################################
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# IngameIME-Interface
add_subdirectory(IngameIME-Common EXCLUDE_FROM_ALL)

# ##############################################################################
# Get Version String
# ##############################################################################
include(GetGitRevisionDescription)
git_describe(VERSION --tags --dirty=-dirty)

configure_file(src/version.cpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)
set(version_file ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)
message(STATUS "Build version: ${VERSION}")

# ##############################################################################
# Build files
# ##############################################################################
aux_source_directory(src SRC_LIST)
list(APPEND SRC_LIST ${version_file})

# ##############################################################################
# Targets
# ##############################################################################

# Library Target
add_library(IngameIME_Static STATIC ${SRC_LIST})
add_library(IngameIME_Shared SHARED ${SRC_LIST})
target_include_directories(IngameIME_Static PRIVATE include)
target_include_directories(IngameIME_Shared PRIVATE include)
target_link_libraries(IngameIME_Static PUBLIC IngameIME)
target_link_libraries(IngameIME_Shared PUBLIC IngameIME)

# ##############################################################################
# Packaging
# ##############################################################################

# Install
install(TARGETS IngameIME_Static IngameIME_Shared)

# Pack
set(CPACK_PACKAGE_FILE_NAME
  ${CMAKE_PROJECT_NAME}-${CMAKE_VS_PLATFORM_NAME}-${VERSION})
set(CPACK_PACKAGE_VENDOR "Windmill_City")
set(CPACK_PACKAGE_CHECKSUM "SHA1")
set(CPACK_GENERATOR "ZIP")

set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pack)

include(CPack)
