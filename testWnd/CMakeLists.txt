cmake_minimum_required(VERSION 3.18)
project(testWnd)

add_subdirectory(glfw EXCLUDE_FROM_ALL)
add_subdirectory(glad EXCLUDE_FROM_ALL)

file(GLOB_RECURSE source_file ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(testWnd ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_include_directories(
    testWnd 
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include 
)
target_sources(testWnd PRIVATE "${source_file}")
target_link_libraries(testWnd PRIVATE glad_static glfw IngameIME_Static)

# Install
install(TARGETS testWnd)
