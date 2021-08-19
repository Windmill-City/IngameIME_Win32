cmake_minimum_required(VERSION 3.0.0)
project(libtf VERSION 0.1.0)

if(NOT MSVC)
    message(FATAL_ERROR "Require MSVC to build!")
endif()

if(MSVC_VERSION LESS 1910)
    message(WARNING "Using legacy MSVC ${MSVC_VERSION}, Suggested version >= 1910")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(FindWindowsSDK)

if(NOT WINDOWSSDK_FOUND)
    message(FATAL_ERROR "No Windows SDK for MSVC ${MSVC_VERSION} found!")
endif()

message(STATUS "Using Windows SDK: ${WINDOWSSDK_LATEST_NAME} at: ${WINDOWSSDK_LATEST_DIR}")

get_windowssdk_include_dirs(${WINDOWSSDK_LATEST_DIR} SDK_INCLUDE_DIR)
get_windowssdk_library_dirs(${WINDOWSSDK_LATEST_DIR} SDK_LIB_DIR)

if((NOT SDK_INCLUDE_DIR) OR (NOT SDK_LIB_DIR))
    message(FATAL_ERROR "Windows SDK missing include/library dir")
endif()

message(STATUS "Windows SDK include dirs: ${SDK_INCLUDE_DIR}")
message(STATUS "Windows SDK library dirs: ${SDK_LIB_DIR}")

include(CTest)
enable_testing()

include(CleanDirectoryList)

file(GLOB_RECURSE SOURCES ./src/**.cpp ./src/**.c)
file(GLOB_RECURSE HEADERS LIST_DIRECTORIES true ./src/**)
clean_directory_list(HEADERS ${PROJECT_SOURCE_DIR}/src ${SDK_INCLUDE_DIR})

message(STATUS "Source Files:${SOURCES}")
message(STATUS "Header Dirs:${HEADERS}")

include_directories(${HEADERS})
link_directories(${SDK_LIB_DIR})
source_group(${SOURCES})

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_library(libtf SHARED src/libtf.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
