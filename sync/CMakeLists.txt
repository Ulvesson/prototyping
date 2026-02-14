# CMakeList.txt : CMake project for sync, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)
project (sync)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source to this project's executable.
add_executable (sync "sync.cpp")
add_executable (renderer "renderer.cpp")

# TODO: Add tests and install targets if needed.
