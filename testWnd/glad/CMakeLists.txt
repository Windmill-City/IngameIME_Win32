cmake_minimum_required(VERSION 3.0.0)
project(glad VERSION 0.1.0)

include(CTest)
enable_testing()

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_library(glad_static STATIC ./src/glad.c)
target_include_directories(glad_static PUBLIC ./include)
target_sources(glad_static PRIVATE ./src)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)