cmake_minimum_required(VERSION 2.8)
project(libMSVL)

set(CMAKE_C_FLAGS "-std=c11 -g -Wall -Wextra -Wshadow -Werror")
set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall -Wextra -Wshadow -Werror")

add_library(dyn_array SHARED dyn_array.c)

add_executable(dyn_array_test dyn_array_tests.cpp)
target_link_libraries(dyn_array_test dyn_array gtest pthread)

add_executable(mytest mytest.c)
target_link_libraries(mytest dyn_array)
