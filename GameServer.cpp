//
// Created by alan on 9/23/17.
//
#include <string>
#include <map>
#include "GameServer.h"
using namespace std; 


#define SERVER_DEBUG 1



session session_bucket[MAX_SESSION];
static map <int, Player> user_map;
//TODO session occupied map used to return a list of activated room
map <int , session*> activated_room;

int udp_callback(void* userptr,const UDPServer::message_t& message,UDPServer::message_t& message_out){
    GameServer * server = reinterpret_cast<GameServer*>(userptr);
    chunk recv = *reinterpret_cast<chunk*>(message.content);
    int player = server->_player_module.getPlayer(recv.pid)->_index;
    message_out.content = server->_session_module.update(recv.sid,
                                                 player,
                                                 message.content,
                                                 message.len);
    message_out.len = message.len;
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
    us.register_cb(udp_callback,this);
    std::thread thread([&us](){
        us.starts();
    });

    TCPServer::init();

    TCPServer::starts();
}

GameServer::GameServer(int udp_port, int tcp_port):TCPServer(tcp_port),_tcp_port(tcp_port),_udp_port(udp_port){};





//TODO don't user double iteration have to change a lot
//Search the session_bucket[] and clr the disconnected fd slot
void GameServer::onShutDownConnection(int fd){
	_player_module.clear(fd);
}

//TODO
void GameServer::onRead(int fd, char * mess, int readsize){
    std::vector<std::string> tokens;
    std::string request_mess(mess,readsize);
    std::istringstream iss(request_mess);
    std::copy(std::istream_iterator<std::string>(iss),
         std::istream_iterator<std::string>(),
         back_inserter(tokens));
#if SERVER_DEBUG
    time_t tt;
    auto now = std::chrono::system_clock::now();
    tt = std::chrono::system_clock::to_time_t(now);
    std::cout << "[" << ctime(&tt) << "] ";
#endif
    std::cout << "##command## " << mess <<std::endl;

    if (tokens[0] == "create"){
        /*message type: create <username>*/
        /*return type: create <username> <pid/fd>*/
        if (tokens.size() < 2) goto paramter_fail;

        int sid = _session_module.create(tokens[1],_player_module.getPlayer(fd)); 	
	    std::string res("created ");
        res += tokens[1] + " " + std::to_string(sid);
        TCPServer::packet_t respond{res.length(), res.c_str()};
        sendPacket(fd, &respond);

    }   else if (tokens[0] == "enter"){
        /*message type: enter <sid> <fd/pid> */
        /*return type: entered <sid> <lobbyname> <entered_player's pid/fd> <etnered_player's username>
         *                     <host_player'pid/fd> <host_player's username> <confirm status bitmap>
         *                     <host_player's cid> <host_player's wid>*/
        /*bitmap usage index 0: host, index1: entered player*/
        if (tokens.size() < 3) goto paramter_fail;

        Player *entered_player = _player_module.getPlayer(fd);

        int sid = std::stoi(tokens[2]);
        sid = _session_module.enter(sid,entered_player);
        std::string res("entered ");
        if (sid) {
            string lobbyname = _session_module.getLobbyName(sid);
            const Player *host_player = _session_module.getPlayer(sid,0);

            res += std::to_string(sid) + " " +
                   lobbyname + " " +
                   std::to_string(fd) + " " +
                   entered_player->_username + " " +
                   std::to_string(host_player->_fd) +
                   host_player->_username + " ";
                   std::to_string(_session_module.confirmState(sid)) +" "+
                   std::to_string(host_player->_cid) + " " +
                   std::to_string(host_player->_wid);

            TCPServer::packet_t respond{res.length(),res.c_str()};
            sendPacket(fd,&respond);
            sendPacket(host_player->_fd,&respond);
        }   else{
            res += " " + std::to_string(sid);
            TCPServer::packet_t respond{res.length(),res.c_str()};
            sendPacket(fd,&respond);
        }

    }   else if (tokens[0] == "exit"){
        /*message type: exit <sid> */
        /*return type: exitted <result>*/

        if (tokens.size() < 2) goto paramter_fail;
        int sid = std::stoi(tokens[2]);
        int index = _player_module.getPlayer(fd)->_index;
        int result = _session_module.exit(sid,index);
        std::string res("exited ");
        res += std::to_string(result);
        TCPServer::packet_t respond{res.length(),res.c_str()};
        sendPacket(fd,&respond);


    }   else if (tokens[0] == "confirm"){
        /*message type: confirm <pid/fd> <sid> <cid> <wid>*/
        /*return type: confirmed <confirmer's pid/fd> <confirm state> <cid> <wid> <o's cid> <p's wid>*/
        if (tokens.size() < 5) goto paramter_fail;
        int pid = std::stoi(tokens[1]);
        int sid = std::stoi(tokens[2]);
        int cid = std::stoi(tokens[3]);
        int wid = std::stoi(tokens[4]);
        const int index = _player_module.getPlayer(pid)->_index;
        std::cout << sid << " " << index <<  " "<< (index^1) << std::endl;
        const Player * oppoent = _session_module.getPlayer(sid,index^1);
        std::cout << oppoent << std::endl;
        int confirm_state = _session_module.confirm(sid,index);
        string res = "confirmed " + std::to_string(fd) + " " + std::to_string(confirm_state);
        res+= " " + std::to_string(cid) + " " + std::to_string(wid);
        int ocid = -1;
        int owid = -1;
        int opponent_fd = -1;
        if (oppoent != nullptr){
            ocid = oppoent->_cid;
            owid = oppoent->_wid;
        }
        res+= " " + std::to_string(ocid) +" " + std::to_string(owid);
        TCPServer::packet_t respond{res.length(),res.c_str()};

        if (oppoent != nullptr) {
            opponent_fd = oppoent->_fd;
            sendPacket(opponent_fd, &respond);
        }

        std::cout << res << std::endl;
        sendPacket(fd,&respond);

        if (_session_module.confirmState(sid) == 3){
            TCPServer::packet_t respond{res.length(),"gamestart"};
            sendPacket(fd,&respond);
            sendPacket(opponent_fd,&respond);
        }

    }   else if (tokens[0] == "start"){
        /*message type: start <sid>*/
        /*return type: start <opponent start?>*/
        /*bitmap usage index 0: host, index1: entered player*/
        if (tokens.size() < 2) goto paramter_fail;
        int sid = std::stoi(tokens[1]);
        const Player* me= _player_module.getPlayer(fd);
        int bitmap = _session_module.startGame(sid,me->_index);
        string res("start");
        res+= " " + std::to_string(bitmap);
        TCPServer::packet_t respond{res.length(),res.c_str()};
        sendPacket(fd,&respond);
    }   else if (tokens[0] == "dead"){
        /*message type: dead <sid>*/
        /*return type: score <score> or win or lose*/
        if (tokens.size() < 2) goto paramter_fail;
        int sid = std::stoi(tokens[1]);
        Player * me = _player_module.getPlayer(fd);
        me->_score++;
        if (me->_score >= 3){
            int opponent_fd = _session_module.getPlayer(sid,1^me->_index)->_fd;
            std::string res("win");
            TCPServer::packet_t respond1{res.length(), res.c_str()};
            sendPacket(opponent_fd, &respond1);
            res = "lose";
            TCPServer::packet_t respond2{res.length(), res.c_str()};
            sendPacket(fd, &respond2);
        }   else {
            int opponent_fd = _session_module.getPlayer(sid,1^me->_index)->_fd;
            std::string res("score");
            TCPServer::packet_t respond{res.length(), res.c_str()};
            sendPacket(opponent_fd, &respond);
        }

    }	else if (tokens[0] == "login"){
        /* message type:login <username> */
        /* return type:login <username> <fd/pid> */
        if (tokens.size() < 2) goto paramter_fail;
        _player_module.record(fd,tokens[1]);

        string res = string(request_mess) + " " + std::to_string(fd);
        TCPServer::packet_t respond{res.length(), res.c_str()};
        sendPacket(fd, &respond);

    }   else if (tokens[0] == "listlobby"){
        /* message type:listlobby <pageno> */
        /* return type: listlobby <list of pair of lobby id and lobbyname> */
        if (tokens.size() < 2) goto paramter_fail;
        int pageno = std::stoi(tokens[1]);
        auto result = _session_module.activatedList(10,pageno);
        string res = "listlobby";
        for (auto iter = result.begin() ; iter != result.end(); ++ iter){
            res = res + " " + std::to_string(iter->first) + " " + iter->second;
        }
        TCPServer::packet_t respond{res.length(), res.c_str()};
        sendPacket(fd, &respond);
    }   else{
paramter_fail:
        string res("paramter invalid");
        TCPServer::packet_t respond{res.length(), "paramter invalid"};
        sendPacket(fd, &respond);
    }
}

//TODO
void GameServer::onAcceptConnection(int fd){
    std::cout << "accept connection" << std::endl;
}
