//
// Created by alan on 9/23/17.
//

#ifndef HELLLOWORLD_GAMESERVER_H
#define HELLLOWORLD_GAMESERVER_H

#include "UDPServer.h"
#include <string>
#include "TCPServer.h"
#include <cstring>
#include <ostream>
#include <ctime>
#include <chrono>
#include <iostream>
#include <thread>
#include <map>

class GameServer:TCPServer {
public:
    GameServer(int udp_port,int tcp_port);
    //int init(std::ostream st);
    void starts();
    virtual void onShutDownConnection(int fd);
    virtual void onRead(int fd, char *, int readsize);
    virtual void onAcceptConnection(int fd);

private:
    //std::ostream log;
    int _udp_port;
    int _tcp_port;

};


#endif //HELLLOWORLD_GAMESERVER_H
