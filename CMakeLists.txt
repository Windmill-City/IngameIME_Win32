cmake_minimum_required(VERSION 3.21.1)
project(IngameIME-Win32 VERSION 3.0.0)

include(IngameIME-Common/IngameIME.cmake)

IngameIME_add_library(IngameIME_Static STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/IngameIME.cpp)
IngameIME_add_library(IngameIME_Shared SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/IngameIME.cpp)

IngameIME_add_swig_library(IngameIME_Java SHARED java)

# Test Window
# add_subdirectory(testWnd testWnd)
