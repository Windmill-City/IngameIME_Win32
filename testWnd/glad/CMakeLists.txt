cmake_minimum_required(VERSION 3.18)
project(glad)

add_library(glad_static STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c)
target_include_directories(glad_static
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(glad_static PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
