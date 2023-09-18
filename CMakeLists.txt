cmake_minimum_required(VERSION 3.25)
project(server2)

set(CMAKE_CXX_STANDARD 17)

add_executable(server2 server.cpp)

# Link against the Winsock library
target_link_libraries(server2 ws2_32)
