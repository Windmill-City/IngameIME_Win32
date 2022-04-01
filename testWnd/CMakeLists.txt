cmake_minimum_required(VERSION 3.18)
project(testWnd)

# ##############################################################################
# Find Packages
# ##############################################################################

add_subdirectory(glfw EXCLUDE_FROM_ALL)
add_subdirectory(glad EXCLUDE_FROM_ALL)

# ##############################################################################
# Build files
# ##############################################################################

file(GLOB_RECURSE source_file ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# ##############################################################################
# Targets
# ##############################################################################

add_executable(testWnd ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_include_directories(testWnd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_sources(testWnd PRIVATE "${source_file}")
target_link_libraries(testWnd PRIVATE glad_static glfw IngameIME_Win32_Static)

# ##############################################################################
# Packaging
# ##############################################################################

# Install
install(TARGETS testWnd)
