#include <iostream>
#include <string>
#include "TCPServer.h"
#include "UDPServer.h"

#define MAX_SESSION 65536
struct pos_t{
    float x;
    float y;
};

struct Player{
    Player(){pos={0,0};};
    Player(pos_t pos){this->pos=pos;};
    int update(pos_t pos){this->pos= pos;};
    pos_t pos;
};

struct chunk{
    pos_t pos;
    int sid;//session_id
    int pid;//0 or 1 at the moment
    chunk(pos_t pos,int sid, int pid){
        this->pos = pos;
        this->sid = sid;
        this->pid = pid;
    }
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
    session_bucket[recv.sid].players[recv.pid].update(recv.pos);
    pos_t enemy_pos = session_bucket[recv.sid].players[recv.pid^1].pos;
    chunk *res = new chunk{enemy_pos,recv.sid,recv.pid^1};
    message_out.content = res;
    message_out.len = sizeof(chunk);
    return 0;
}