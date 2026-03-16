cmake_minimum_required(VERSION 3.10.0)
project(tiny_log VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-O3)

add_executable(tiny_log main.cpp)

target_link_directories(tiny_log PRIVATE ${CMAKE_PROJECT_PATH})