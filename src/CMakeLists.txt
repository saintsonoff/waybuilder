cmake_minimum_required(VERSION 3.30)

set(PROJECT_NAME labwork6)

include(CMakeProjSettings.cmake)

project(${PROJECT_NAME})

add_subdirectory(additions)
add_subdirectory(lib)
add_subdirectory(bin)
