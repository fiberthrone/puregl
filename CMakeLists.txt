cmake_minimum_required(VERSION 3.10)
project(puregl VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (WIN32)
    set(CMAKE_C_FLAGS_RELEASE "/O2")
else()
    set(CMAKE_C_FLAGS_RELEASE "-O2")
endif()

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(src/third_party/glfw)

if(NOT WIN32)
    set(GLAD_LIBRARIES dl)
endif()

include_directories(src/third_party/glfw/include/
                    src/third_party/glad/include/
                    src/third_party/cglm/include/)

add_executable(puregl src/puregl.c src/third_party/glad/src/glad.c)

target_link_libraries(puregl glfw ${GLAD_LIBRARIES})
