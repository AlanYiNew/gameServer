#include <iostream>
#include <string>
#include "TCPServer.h"
#include <cstring>
#include "UDPServer.h"
#include <ctime>
#include <chrono>

#define MAX_SESSION 65536
/*
struct pos_t{
    float x;
    float y;
};
*/

struct Player{
    void * data;
    int len;
    Player():data(nullptr),len(0){};
};

struct chunk{
    int sid;
    int pid;
};


struct session{
    Player players[2];
    int sid;
};

session session_bucket[MAX_SESSION];

int function_callback(const UDPServer::message_t& message,UDPServer::message_t& message_out);

int main(int argc, char * argv[]){
    UDPServer us(8666);
    us.init();
    us.register_cb(function_callback);
    us.starts();
    return 0;
}

int function_callback(const UDPServer::message_t& message,UDPServer::message_t& message_out){
    chunk recv = *reinterpret_cast<chunk*>(message.content);
    if (session_bucket[recv.sid].players[recv.pid].data == nullptr){
        session_bucket[recv.sid].players[recv.pid].data = malloc(message.len);
    }

    memcpy(session_bucket[recv.sid].players[recv.pid].data,message.content,message.len);
    session_bucket[recv.sid].players[recv.pid].len = message.len;

    message_out.content = session_bucket[recv.sid].players[recv.pid^1].data;
    message_out.len = session_bucket[recv.sid].players[recv.pid^1].len;

    time_t tt;
    auto now = std::chrono::system_clock::now();
    tt = std::chrono::system_clock::to_time_t(now);

    std::cout << "recving " << message.len << " from "<<recv.pid << "[" << ctime(&tt) << "]"<< std::endl;
    return 0;
}
