cmake_minimum_required(VERSION 3.21.1)
project(testWnd VERSION 0.1.0)

add_subdirectory(glfw ./glfw)
add_subdirectory(glad ./glad)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(testWnd ./src/main.cpp)
target_include_directories(testWnd PRIVATE ../src/include ./glfw/include ./glad/include)
target_sources(testWnd PRIVATE ./src)
target_link_libraries(testWnd PRIVATE glad_static)
target_link_libraries(testWnd PRIVATE glfw ${GLFW_LIBRARIES})
target_link_libraries(testWnd PRIVATE libtf_static)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
