cmake_minimum_required(VERSION 3.10.0)
project(optional VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall -Werror -Wextra)

add_executable(optional main.cpp)

