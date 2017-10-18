//
// Created by alan on 9/23/17.
//
#include <string>
#include <map>
#include "GameServer.h"
#define MAX_SESSION 8
#define MAX_PLAYERS 2

using namespace std; 


#define SERVER_DEBUG 0
struct Player{
    void * data;
    int len;
    int fd;//file descriptor
    bool confirmed;
    bool starts;
    int score;
    string username;//TODO name is here
    Player():data(nullptr),len(0),confirmed(0),starts(0),username("Noob"){};
    Player(string u):data(nullptr),len(0),confirmed(0),starts(0),username(u){};
};

struct chunk{
    int sid;
    int pid;
};

struct session{
    Player* players[2];
    int occupied;
};

int nextfree = 0;
int available = MAX_SESSION;
session session_bucket[MAX_SESSION];
static map <int, Player> user_map;
//TODO session occupied map used to return a list of activated room
map <int , session*> activated_room;

int udp_callback(const UDPServer::message_t& message,UDPServer::message_t& message_out){
    chunk recv = *reinterpret_cast<chunk*>(message.content);
    if (session_bucket[recv.sid].players[recv.pid]->data == nullptr){
        session_bucket[recv.sid].players[recv.pid]->data = malloc(message.len);
    }

    memcpy(session_bucket[recv.sid].players[recv.pid]->data,message.content,message.len);
    session_bucket[recv.sid].players[recv.pid]->len = message.len;

    message_out.content = session_bucket[recv.sid].players[recv.pid^1]->data;
    message_out.len = session_bucket[recv.sid].players[recv.pid^1]->len;

#if SERVER_DEBUG
    time_t tt;
    auto now = std::chrono::system_clock::now();
    tt = std::chrono::system_clock::to_time_t(now);

    std::cout << "recving " << message.len << " from "<<recv.pid << "[" << ctime(&tt) << "]"<< std::endl;
#endif
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

    session_bucket[nextfree].occupied++;
    session_bucket[nextfree].players[0]->fd = fd;
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
    
    std::cout << "enter checking" << session_bucket[sid].occupied << std::endl;
    if (sid >= 0 && sid < MAX_SESSION
        && session_bucket[sid].occupied){
        session_bucket[sid].players[1]->fd = fd;
        session_bucket[sid].occupied++;
        return true;
    } 
    return false;
}

bool validSid(int sid){
    return sid >= 0 && sid < MAX_SESSION
        && session_bucket[sid].occupied;
}

void clear_player(int i,int j){
    free(session_bucket[i].players[j]->data);
	session_bucket[i].players[j]->data = nullptr;
	session_bucket[i].players[j]->len = 0;
	session_bucket[i].players[j]->fd = 0;
	session_bucket[i].players[j]->confirmed = false;
	session_bucket[i].players[j]->starts = false;
	session_bucket[i].occupied--;
	std::cout << "Session: "<< i << " Player: " << j <<
		" has been cleared!" << std::endl;			
}

//TODO don't user double iteration
//Search the session_bucket[] and clr the disconnected fd slot
void GameServer::onShutDownConnection(int fd){
	//iteral all session_bucket slots, and check players' fd value
	//if value is equal to the parrameter 'fd', then remove the player's data
	for ( int i = 0; i < MAX_SESSION; i++ ){
		int num_of_empty_players = 0; //not empty initiall
		for ( int j = 0; j < MAX_PLAYERS; j++ ){
			if ( session_bucket[i].players[j]->fd == fd ){
				clear_player(i,j);
				string user = user_map[fd].username;
				user_map.erase(fd);// clear the username in the username_map if user leaves
				cout << "User " << user << "has been removed from username_map" << endl;
			}
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
        //TODO add username
        int sid = nextfit(fd);
        if (sid >= 0) {
            session_bucket[sid].players[0] = &user_map[fd];
            activated_room[sid] = &session_bucket[sid];
        }

        std::string res("created ");
        res += std::to_string(sid);
        TCPServer::packet_t respond{res.length(), res.c_str()};
        sendPacket(fd, &respond);
    }   else if (tokens[0] == "enter" && tokens[1] == "lobby"){
        //TODO add username and don't user both
        int sid = std::stoi(tokens[2]);
        std::string res("entered ");
        bool found = enterSession(sid,fd);
        if (found) session_bucket[sid].players[1] = &user_map[fd];
        res+=std::to_string(found?sid:-1);

        TCPServer::packet_t respond{res.length(),res.c_str()};
        sendPacket(fd,&respond);
        int opponent_fd = session_bucket[sid].players[0]->fd;
        res = "both";
        respond = {res.length(),res.c_str()};
        sendPacket(opponent_fd,&respond);

    }   else if (tokens[0] == "exit" && tokens[1] == "lobby"){
        int sid = std::stoi(tokens[2]);
	    int pid = std::stoi(tokens[3]);
        std::string res("exited lobby ");



        if (validSid(sid)){
               clear_player(sid,pid);
               session_bucket[sid].occupied--;
               session_bucket[sid].players[pid] = NULL;
               if (session_bucket[sid].occupied==0){
                   activated_room.erase(sid);
               }
               res+=std::to_string(sid);
        }	else{
           res+="-1";
        }

        TCPServer::packet_t respond{res.length(),res.c_str()};
        sendPacket(fd,&respond);


    }   else if (tokens[0] == "confirm"){
        //TODO refractor logic
        int sid = std::stoi(tokens[1]);
        int pid = std::stoi(tokens[2]);
        int cid = std::stoi(tokens[3]);
        int wid = std::stoi(tokens[4]);
        session_bucket[sid].players[pid]->confirmed = true;

        std::string res("opponent confirmed ");
        res+= std::to_string(cid) + " " + std::to_string(wid);
        TCPServer::packet_t respond{res.length(),res.c_str()};
        int opponent_fd = session_bucket[sid].players[pid^1]->fd;
        sendPacket(opponent_fd,&respond);

    }   else if (tokens[0] == "start"){
        //TODO refractor and reimplement
        int sid = std::stoi(tokens[1]);
        int pid = std::stoi(tokens[2]);
        session_bucket[sid].players[pid]->starts = true;
        if (session_bucket[sid].players[pid^1]->starts){

            std::string res("game start");
            TCPServer::packet_t respond{res.length(),res.c_str()};
            int opponent_fd = session_bucket[sid].players[pid^1]->fd;
            sendPacket(opponent_fd,&respond);
            sendPacket(fd,&respond);

        }   else{
            std::string res("opponent not ready");
            TCPServer::packet_t respond{res.length(),res.c_str()};
            std::cout << "len" <<  sendPacket(fd,&respond) << std::endl;
        }
    }   else if (tokens[0] == "dead"){
        int sid = std::stoi(tokens[1]);
        int pid = std::stoi(tokens[2]);

        session_bucket[sid].players[pid^1]->score++;
        if (session_bucket[sid].players[pid^1]->score >= 3){
            int opponent_fd = session_bucket[sid].players[pid ^ 1]->fd;
            std::string res("win");
            TCPServer::packet_t respond1{res.length(), res.c_str()};
            sendPacket(opponent_fd, &respond1);
            res = "lose";
            TCPServer::packet_t respond2{res.length(), res.c_str()};
            sendPacket(fd, &respond2);
        }   else {
            int opponent_fd = session_bucket[sid].players[pid ^ 1]->fd;
            std::string res("score");
            TCPServer::packet_t respond{res.length(), res.c_str()};
            sendPacket(opponent_fd, &respond);
        }

        std::cout << "after " << sid << " " << pid << " " << session_bucket[sid].players[pid^1]->score << std::endl;
    }	else if (tokens[0] == "login"){
        //TODO change
        user_map.emplace({fd,{tokens[1]}});
    	//add pair<fd, username> to the username_map, suppose tokens[1] is username
    	user_map[fd] = tokens[1];
    	cout << "username_map[" << fd << "] = " << tokens[1] << endl;
    	
    }   else if (tokens[0] == "lobby" && tokens[1] == "list"){
        //TODO return a list of lobby
        int pageno = std::stoi(tokens[2]);
    }
}

//TODO
void GameServer::onAcceptConnection(int fd){
    std::cout << "accept connection" << std::endl;
}
