cmake_minimum_required(VERSION 3.10.0)
project(new_features VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-O2)

add_executable(new_features main.cpp)

