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
add_subdirectory(IngameIME-Common)

# ##############################################################################
# Get Version String
# ##############################################################################

include(GetGitRevisionDescription)
git_describe(VERSION --tags --dirty=-dirty)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)
set(version_file ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)
message(STATUS "Build version: ${VERSION}")

# ##############################################################################
# Build files
# ##############################################################################

set(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE source_file ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(APPEND source_file ${version_file})

# ##############################################################################
# Targets
# ##############################################################################

# Library Target
add_library(IngameIME_Win32_Static STATIC "${source_file}")
add_library(IngameIME_Win32_Shared SHARED "${source_file}")
target_include_directories(IngameIME_Win32_Static PRIVATE "${include_dir}")
target_include_directories(IngameIME_Win32_Shared PRIVATE "${include_dir}")
target_link_libraries(IngameIME_Win32_Static PUBLIC IngameIME_Interface)
target_link_libraries(IngameIME_Win32_Shared PUBLIC IngameIME_Interface)

# ##############################################################################
# Packaging
# ##############################################################################

# Install
install(TARGETS IngameIME_Win32_Static IngameIME_Win32_Shared
                IngameIME_Interface)

# Pack
set(CPACK_PACKAGE_FILE_NAME
    ${CMAKE_PROJECT_NAME}-${CMAKE_VS_PLATFORM_NAME}-${VERSION})
set(CPACK_PACKAGE_VENDOR "Windmill_City")
set(CPACK_PACKAGE_CHECKSUM "SHA1")
set(CPACK_GENERATOR "ZIP")

set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pack)

include(CPack)
