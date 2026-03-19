cmake_minimum_required(VERSION 3.10.0)
project(json VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
add_executable(json main.cpp)

target_link_libraries(json PRIVATE jsoncpp)