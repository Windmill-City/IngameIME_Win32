cmake_minimum_required(VERSION 3.18)
project(testWnd VERSION 0.1.0)

add_subdirectory(glfw ${CMAKE_CURRENT_SOURCE_DIR}/glfw)
add_subdirectory(glad ${CMAKE_CURRENT_SOURCE_DIR}/glad)

add_executable(testWnd ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_include_directories(testWnd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)
target_sources(testWnd PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(testWnd PRIVATE glad_static)
target_link_libraries(testWnd PRIVATE glfw ${GLFW_LIBRARIES})
target_link_libraries(testWnd PRIVATE IngameIME_Static)
