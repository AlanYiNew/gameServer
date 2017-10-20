//
// Created by alan on 9/23/17.
//

#ifndef HELLLOWORLD_GAMESERVER_H
#define HELLLOWORLD_GAMESERVER_H

#include "UDPServer.h"
#include <string>
#include "TCPServer.h"
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <unordered_set>
#include <thread>
#include <map>
#include <algorithm>
#include <utility>
#include <iterator>
#include <sstream>
#include "Modules.h"
#include "gs_log.h"

class GameServer:TCPServer {
    friend int udp_callback(void* userptr,const UDPServer::message_t& message,UDPServer::message_t& message_out);

public:
    GameServer(int udp_port,int tcp_port,const string & sc_url);
    //int init(std::ostream st);
    void starts();
    virtual void onShutDownConnection(int fd);
    virtual void onRead(int fd, char *, int readsize);
    virtual void onAcceptConnection(int fd);


private:
    //std::ostream log;
    int _udp_port;
    int _tcp_port;
    PlayerModule _player_module;
    SessionModule _session_module;
    GS_LOG log;
    int send_respond(int fd, const std::unordered_map<string,string>& map);
    int send_respond(int fd, const std::map<int,string>& map);
    bool is_alive(int fd);
    SCChecker _sanity_check;

};

class SCChecker{
private:
    std::unordered_map<string,std::unordered_set<string>> sc;
public:
    SCChecker(ifstream & ifs);
    bool isValid(std::unordered_map<string,string> &req);
};

#endif //HELLLOWORLD_GAMESERVER_H
