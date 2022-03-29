cmake_minimum_required(VERSION 3.18)
project(IngameIME-Win32)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Appends the cmake/modules path to MAKE_MODULE_PATH variable.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

include(GetGitRevisionDescription)
git_describe(VERSION --tags --dirty=-dirty)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in
                ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)
set(version_file ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)
message(STATUS "Build version: ${VERSION}")

include(IngameIME-Common/IngameIME.cmake)

set(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
file(GLOB_RECURSE source_file ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(APPEND source_file ${version_file})

IngameIME_add_library(IngameIME_Static STATIC ${source_file})
IngameIME_add_library(IngameIME_Shared SHARED ${source_file})
target_include_directories(IngameIME_Static PRIVATE "${include_dir}")
target_include_directories(IngameIME_Shared PRIVATE "${include_dir}")

# Test Window
add_subdirectory(testWnd)

# Install
install(TARGETS IngameIME_Static IngameIME_Shared testWnd PUBLIC_HEADER)

# Pack
set(CPACK_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-${CMAKE_VS_PLATFORM_NAME}-${VERSION})
set(CPACK_PACKAGE_VENDOR "Windmill_City")
set(CPACK_PACKAGE_CHECKSUM "SHA1")
set(CPACK_GENERATOR "ZIP")

set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pack)

include(CPack)
