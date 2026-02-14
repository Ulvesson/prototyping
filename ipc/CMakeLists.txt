cmake_minimum_required(VERSION 3.15)
project(IPCDemo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find threads package
find_package(Threads REQUIRED)

# Server application
add_executable(Server
    server.cpp
)

# Client application
add_executable(Client
    client.cpp
)

# Link libraries
target_link_libraries(Server PRIVATE Threads::Threads)

if(WIN32)
    target_link_libraries(Server PRIVATE ws2_32)
    target_link_libraries(Client PRIVATE ws2_32)
else()
    target_link_libraries(Server PRIVATE pthread)
endif()

# Set output directory
set_target_properties(Server Client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
