cmake_minimum_required(VERSION 3.21.1)
project(libtf VERSION 3.0.0)

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
clean_directory_list(PRIVATE_HEADERS ${PROJECT_SOURCE_DIR}/src ${SDK_INCLUDE_DIR})

message(STATUS "Source Files:${SOURCES}")
message(STATUS "Private Header Dirs:${PRIVATE_HEADERS}")

set(STATIC_VC_RUNTIME 0)

# Compiler Option
if(STATIC_VC_RUNTIME)
    include(MSVCStaticRuntime)
endif()
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Target
add_library(libtf_static STATIC src/libtf.cpp)
target_include_directories(libtf_static PUBLIC ./src/include)
target_include_directories(libtf_static PRIVATE ${PRIVATE_HEADERS})
target_link_directories(libtf_static PRIVATE ${SDK_LIB_DIR})
target_sources(libtf_static PRIVATE ${SOURCES})

add_library(libtf_shared SHARED src/libtf.cpp)
target_include_directories(libtf_shared PUBLIC ./src/include)
target_include_directories(libtf_shared PRIVATE ${PRIVATE_HEADERS})
target_link_directories(libtf_shared PRIVATE ${SDK_LIB_DIR})
target_sources(libtf_shared PRIVATE ${SOURCES})

# Test Window
add_subdirectory(testWnd testWnd)

# Install
set(CMAKE_INSTALL_PREFIX ./install)
install(TARGETS libtf_static)
install(TARGETS libtf_shared)
install(DIRECTORY src/include/ TYPE INCLUDE)

# Package
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Windmill_City")
set(CPACK_GENERATOR "ZIP")
include(CPack)
