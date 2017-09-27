//
// Created by alan on 9/23/17.
//

#include "GameServer.h"
#define MAX_SESSION 65536
#define MAX_PLAYERS 2

struct Player{
    void * data;
    int len;
    int fd;//file descriptor
    bool confirmed;
    bool starts;
    Player():data(nullptr),len(0),confirmed(0),starts(0){};
};

struct chunk{
    int sid;
    int pid;
};

struct session{
    Player players[2];
    bool occupied;
};

int nextfree = 0;
int available = MAX_SESSION;
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

int nextfit(int fd){
    if (!available) return -1;

    session_bucket[nextfree].occupied=true;
    session_bucket[nextfree].players[0].fd = fd;
    int result = nextfree;
    available--;
    while (available) {
        nextfree= (nextfree+1)%MAX_SESSION;
        if (!session_bucket[nextfree].occupied) {
            break;
        }
    }
    return result;
}

bool enterSession(int sid,int fd){
    if (sid >= 0 && sid < MAX_SESSION
        && session_bucket[sid].occupied){
        session_bucket[sid].players[1].fd = fd;
        return true;
    }
    return false;
}

bool validSid(int sid){
    return sid >= 0 && sid < MAX_SESSION
        && session_bucket[sid].occupied;
}

//Search the session_bucket and clr the disconnected fd slot
void GameServer::onShutDownConnection(int fd){
	//iteral all session_bucket slots, and check players' fd value
	//if value is equal to the parrameter 'fd', then remove the player's data
	for ( int i = 0; i < MAX_SESSION; i++ ){
		for ( int j = 0; j < MAX_PLAYERS; j++ ){
			if ( session_bucket[i].players[j].fd == fd ){
				senssion_bucket[i].players[j].data = nullptr;
				senssion_bucket[i].players[j].len = 0;
				senssion_bucket[i].players[j].fd = 0;
				senssion_bucket[i].players[j].confirmed = false;
				senssion_bucket[i].players[j].starts = false;
			}
		}
		//if the data of all players' been removed, then room occupied is set to 'false'
		if ( senssion_bucket[i].players[0].starts = false && 
		  senssion_bucket[i].players[1].starts = false){
			senssion_bucket[i].occupied = false;
		}
	}
}

//TODO
void GameServer::onRead(int fd, char * mess, int readsize){
    std::vector<std::string> tokens;
    std::string str(mess,readsize);
    std::istringstream iss(str);
    std::copy(std::istream_iterator<std::string>(iss),
         std::istream_iterator<std::string>(),
         back_inserter(tokens));
    std::cout << "##command## " << mess <<std::endl;
    if (tokens[0] == "create" && tokens[1] == "lobby"){
        int sid = nextfit(fd);
        std::string res("created ");
        res+=std::to_string(sid);
        TCPServer::packet_t respond{res.length(),res.c_str()};
        sendPacket(fd,&respond);

    }   else if (tokens[0] == "enter" && tokens[1] == "lobby"){
        int sid = std::stoi(tokens[2]);
        std::string res("entered ");
        res+=std::to_string((enterSession(sid,fd)?sid:-1));
        TCPServer::packet_t respond{res.length(),res.c_str()};
        sendPacket(fd,&respond);

    }   else if (tokens[0] == "exit" && tokens[1] == "lobby"){
        int sid = std::stoi(tokens[2]);
        std::string res("exited lobby ");
        res+=std::to_string((validSid(sid)?sid:-1));
        TCPServer::packet_t respond{res.length(),res.c_str()};
        sendPacket(fd,&respond);

    }   else if (tokens[0] == "confirm"){
        int sid = std::stoi(tokens[1]);
        int pid = std::stoi(tokens[2]);
        int cid = std::stoi(tokens[3]);
        int wid = std::stoi(tokens[4]);
        session_bucket[sid].players[pid].confirmed = true;

        std::string res("opponent confirmed ");
        res+= std::to_string(cid) + " " + std::to_string(wid);
        TCPServer::packet_t respond{res.length(),res.c_str()};
        int opponent_fd = session_bucket[sid].players[pid^1].fd;
        sendPacket(opponent_fd,&respond);

    }   else if (tokens[0] == "start"){
        int sid = std::stoi(tokens[1]);
        int pid = std::stoi(tokens[2]);
        session_bucket[sid].players[pid].starts = true;
        if (session_bucket[sid].players[pid^1].starts){

            std::string res("game start");
            TCPServer::packet_t respond{res.length(),res.c_str()};
            int opponent_fd = session_bucket[sid].players[pid^1].fd;
            sendPacket(opponent_fd,&respond);
            std::cout << "len" << sendPacket(fd,&respond) << std::endl;

        }   else{
            std::string res("opponent not ready");
            TCPServer::packet_t respond{res.length(),res.c_str()};
            std::cout << "len" <<  sendPacket(fd,&respond) << std::endl;
        }
    }
}

//TODO
void GameServer::onAcceptConnection(int fd){
    std::cout << "accept connection" << std::endl;
}
