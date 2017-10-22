cmake_minimum_required(VERSION 3.5)
project(gameServer)

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
set(SOURCE_FILES main.cpp network/TCPServer.cpp network/TCPServer.h network/UDPServer.cpp network/UDPServer.h GameServer.cpp GameServer.h modules/Modules.h logsys/gs_log.cpp logsys/gs_log.h utils/utils.cpp utils/utils.h modules/Map.h modules/Map.cpp modules/Game.cpp modules/Game.h modules/Session.cpp modules/Session.h modules/Player.cpp modules/Player.h)
add_executable(gameServer ${SOURCE_FILES})
