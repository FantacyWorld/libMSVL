cmake_minimum_required(VERSION 2.8)
project(libMSVL)

set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wshadow -Werror")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Werror")

add_library(dyn_arr SHARED dyn_array.c)

add_executable(${PROJECT_NAME}_test tests.cpp)
target_link_libraries(${PROJECT_NAME}_test dyn_arr gtest pthread)
