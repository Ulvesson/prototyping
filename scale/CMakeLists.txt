cmake_minimum_required(VERSION 3.10)

project(Scale)
#Set C++ standard
set(CMAKE_CXX_STANDARD 17)
add_executable(scale_test test.cpp)
target_link_libraries(scale_test PRIVATE stdc++fs)
