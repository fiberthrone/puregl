cmake_minimum_required(VERSION 3.10)
project(puregl VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(src/third_party/glfw)

include_directories(src/third_party/glfw/include/)

add_executable(puregl src/puregl.c)

target_link_libraries(puregl glfw)
