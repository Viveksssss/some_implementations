cmake_minimum_required(VERSION 3.10.0)
project(co_async VERSION 0.1.0 LANGUAGES C CXX)

add_executable(co_async main.cpp)

find_library(LIBURING_LIB uring REQUIRED)

target_include_directories(co_async PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(co_async PRIVATE uring)
