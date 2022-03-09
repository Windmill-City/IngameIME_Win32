cmake_minimum_required(VERSION 3.18)
project(testWnd)

add_subdirectory(glfw EXCLUDE_FROM_ALL)
add_subdirectory(glad EXCLUDE_FROM_ALL)

add_executable(testWnd ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_sources(testWnd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(testWnd PRIVATE glad_static glfw IngameIME_Static)

# Install
install(TARGETS testWnd)
