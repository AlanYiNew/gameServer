//
// Created by alan on 9/23/17.
//

#include "GameServer.h"
#define MAX_SESSION 65536
struct Player{
    void * data;
    int len;
    int fd;//file descriptor
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

int udp_callback(const UDPServer::message_t& message,UDPServer::message_t& message_out){
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

void GameServer::starts() {
    UDPServer us(_udp_port);
    us.init();
    us.register_cb(udp_callback);
    std::thread thread([&us](){
        us.starts();
    });

    TCPServer::init();

    TCPServer::starts();
}

GameServer::GameServer(int udp_port, int tcp_port):TCPServer(tcp_port),_tcp_port(tcp_port),_udp_port(udp_port){};

//TODO
void GameServer::onShutDownConnection(int fd){

}

//TODO
void GameServer::onRead(int fd, char *, int readsize){

}

//TODO
void GameServer::onAcceptConnection(int fd){

}
