cmake_minimum_required(VERSION 3.18)
project(IngameIME-Win32)

set(CMAKE_CXX_STANDARD 17)

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
set(source_file ${CMAKE_CURRENT_SOURCE_DIR}/src/IngameIMEImpl.cpp ${version_file})

IngameIME_add_library(IngameIME_Static STATIC ${source_file})
IngameIME_add_library(IngameIME_Shared SHARED ${source_file})
target_include_directories(IngameIME_Static PRIVATE ${include_dir})
target_include_directories(IngameIME_Shared PRIVATE ${include_dir})

IngameIME_add_swig_library(IngameIME_Java SHARED java ${source_file})
target_include_directories(IngameIME_Java PRIVATE ${include_dir})

# Test Window
add_subdirectory(testWnd testWnd)
