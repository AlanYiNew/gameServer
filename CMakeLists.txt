cmake_minimum_required(VERSION 3.5)
project(Hellloworld)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(SOURCE_FILES main.cpp TCPServer.cpp TCPServer.h UDPServer.cpp UDPServer.h GameServer.cpp GameServer.h)
add_executable(gameServer ${SOURCE_FILES})
