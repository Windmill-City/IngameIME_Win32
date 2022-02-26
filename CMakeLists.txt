cmake_minimum_required(VERSION 3.18)
project(IngameIME-Win32 VERSION 5.1.0)

set(CMAKE_CXX_STANDARD 17)

include(IngameIME-Common/IngameIME.cmake)

IngameIME_add_library(IngameIME_Static STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/IngameIMEImpl.cpp)
IngameIME_add_library(IngameIME_Shared SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/IngameIMEImpl.cpp)
target_include_directories(IngameIME_Static PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
target_include_directories(IngameIME_Shared PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include)

# IngameIME_add_swig_library(IngameIME_Java SHARED java)
# target_include_directories(IngameIME_Java PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include)
# target_sources(IngameIME_Java PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/IngameIMEImpl.cpp)

# Test Window
add_subdirectory(testWnd testWnd)
