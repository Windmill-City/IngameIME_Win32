cmake_minimum_required(VERSION 3.21.1)
project(IngameIME-WINDOWS VERSION 3.0.0)

file(GLOB_RECURSE IngameIME_PRIVATE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/**.c)

set(IngameIME_PRIVATE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
set(IngameIME_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/IngameIME.cpp)

add_subdirectory(IngameIME-Common IngameIME-Common)

# Test Window
add_subdirectory(testWnd testWnd)
