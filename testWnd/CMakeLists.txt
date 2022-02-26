cmake_minimum_required(VERSION 3.18)
project(testWnd)

add_subdirectory(glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw EXCLUDE_FROM_ALL)
add_subdirectory(glad ${CMAKE_CURRENT_BINARY_DIR}/glad EXCLUDE_FROM_ALL)

add_executable(testWnd ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_include_directories(testWnd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)
target_sources(testWnd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(testWnd PRIVATE glad_static)
target_link_libraries(testWnd PRIVATE glfw ${GLFW_LIBRARIES})
target_link_libraries(testWnd PRIVATE IngameIME_Static)

# Install
install(TARGETS testWnd)
