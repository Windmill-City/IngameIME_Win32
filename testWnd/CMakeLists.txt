cmake_minimum_required(VERSION 3.0.0)
project(testWnd VERSION 0.1.0)

include(CTest)
enable_testing()

add_subdirectory(../ ./libtf)
add_subdirectory(glfw ./glfw)

include_directories(../src/ ./glfw/include)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(testWnd main.cpp)
target_link_libraries(testWnd glfw ${GLFW_LIBRARIES})
target_link_libraries(testWnd libtf)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
