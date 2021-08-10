cmake_minimum_required(VERSION 3.0.0)
project(testWnd VERSION 0.1.0)

include(CTest)
enable_testing()

link_libraries(glfw)
link_libraries(libtf)

add_executable(testWnd main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
