cmake_minimum_required(VERSION 3.5)
project(Hellloworld)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp TCPServer.cpp TCPServer.h UDPServer.cpp UDPServer.h)
add_executable(Hellloworld ${SOURCE_FILES})