//
// Created by alan on 9/23/17.
//
#include <string>
#include <map>
#include "GameServer.h"

using namespace std; 


#define SERVER_DEBUG 1

std::unordered_map<string,string> req_parse(const string& mess);
string res_parse(const std::unordered_map<string,string>& map);





int udp_callback(void* userptr,const UDPServer::message_t& message,UDPServer::message_t& message_out){
    GameServer * server = reinterpret_cast<GameServer*>(userptr);
    chunk recv = *reinterpret_cast<chunk*>(message.content);
    server->_player_module.update(recv.pid,message.content,message.len);
    int opponent_fd = server->_session_module.getOpponent(recv.sid,recv.pid);

    message_out.content = server->_player_module.getPlayer(opponent_fd)->_data;
    message_out.len = message.len;
#if SERVER_DEBUG
    server->log.LOG("recving " + std::to_string(message.len) +" from " +  std::to_string(recv.pid));
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

GameServer::GameServer(int udp_port, int tcp_port):TCPServer(tcp_port),_tcp_port(tcp_port),_udp_port(udp_port),log(std::cout){};

void GameServer::onShutDownConnection(int fd){
	_player_module.clear(fd);
}

void GameServer::onRead(int fd, char * mess, int readsize){

    std::string request_mess(mess,readsize);

#if SERVER_DEBUG
    log.LOG("### command ### " + request_mess);
#endif
    auto req = req_parse(request_mess);

    if (req["cmd"] == "create"){
        /*message type: create <lobbyname>*/
        /*return type: create <lobbyname> <pid/fd>*/
        int sid = _session_module.create(req["lobbyname"],fd);

        std::unordered_map<string,string> res;

        res["cmd"] = "create";
        res["lobbyname"] = req["lobbyname"];
        res["sid"] = to_string(sid);
        res["success"] = to_string(_session_module.validSid(sid)?0:-1);
        res["pid"] = to_string(fd);

        send_respond(fd,res);

    }   else if (req["cmd"] == "enter"){

        int sid = std::stoi(req["sid"]);
        Player *entered_player = _player_module.getPlayer(fd);
        sid = _session_module.enter(sid,fd);
        std::unordered_map<string,string> res;

        if (_session_module.validSid(sid)) {
            const string &lobbyname = _session_module.getLobbyName(sid);
            const int host_player_fd = _session_module.getOpponent(sid,fd);
            const Player *host_player = _player_module.getPlayer(host_player_fd);

            res["cmd"] = "enter";
            res["sid"] = std::to_string(sid);
            res["lobbyname"] = lobbyname;
            res["pid1"] = to_string(host_player_fd);
            res["username1"] = host_player->_username;
            res["cid1"] = to_string(host_player->_cid);
            res["wid1"] = to_string(host_player->_wid);
            res["pid2"] = to_string(fd);
            res["username2"] = entered_player->_username;
            res["cid2"] = to_string(entered_player->_cid);
            res["wid2"] = to_string(entered_player->_wid);
            res["success"] = "0";


            send_respond(fd,res);
            send_respond(host_player_fd,res);
        }   else{

            res["success"] = "-1";
            send_respond(fd,res);
        }

    }   else if (req["cmd"] == "exit"){
        /*message type: exit <sid> */
        /*return type: exitted <result>*/

        int sid = std::stoi(req["sid"]);

        int result = _session_module.exit(sid,fd);
        std::unordered_map<string,string> res;
        int opponenet_fd =  _session_module.getOpponent(sid,fd);

        res["pid"] = to_string(fd);
        res["cmd"] = "exit";
        if (result >= 0){
            res["success"] = "0";
        }   else{
            res["success"] = "-1";
        }

        send_respond(fd,res);
        if (opponenet_fd > 2)
            send_respond(opponenet_fd,res);

    }   else if (req["cmd"] == "confirm"){
        /*message type: confirm <pid/fd> <sid> <cid> <wid>*/
        /*return type: confirmed <confirmer's pid/fd> <ready state> <cid> <wid> <o's cid> <p's wid>*/

        int pid = fd;
        int sid = std::stoi(req["sid"]);
        int cid = std::stoi(req["cid"]);
        int wid = std::stoi(req["wid"]);
        const int opponent_fd = _session_module.getOpponent(sid,fd);
        std::unordered_map<string,string> res;

        bool ready_state = _player_module.confirm(fd,wid,cid);

        res["cmd"] = "confirm";
        res["pid"] = to_string(fd);
        res["readystate"] = to_string(ready_state);
        res["cid"] = to_string(wid);
        res["wid"] = to_string(cid);
        res["success"] = "0";

        if (opponent_fd > 0) {
            send_respond(opponent_fd,res);
        }
        send_respond(fd,res);

        if (_player_module.getPlayer(fd)->_confirmed
            && _player_module.getPlayer(opponent_fd)->_confirmed){
            res["cmd"] = "cmd gamestart";
            res["success"] = "0";
            send_respond(fd,res);
            send_respond(opponent_fd,res);
        }

    }   else if (req["cmd"] == "dead"){
        /*message type: dead <sid>*/
        /*return type: score <score> or win or lose*/

        std::unordered_map<string,string> res;
        int sid = std::stoi(req["sid"]);
        int opponent_fd = _session_module.getOpponent(sid,fd);
        Player * p = _player_module.getPlayer(opponent_fd);
        p->_score++;
        res["success"] = "0";
        if (p->_score >= 3){
            res["cmd"] = "win";
            send_respond(opponent_fd,res);
            res["cmd"] = "lose";
            send_respond(fd,res);
        }   else {

            res["cmd"] = "score";
            res["score"] = to_string(p->_score);
            send_respond(opponent_fd,res);
        }

    }	else if (req["cmd"] == "login"){
        /* message type:login <username> */
        /* return type:login <username> <fd/pid> */

        _player_module.record(fd,req["username"]);
        std::unordered_map<string,string> res;

        res["success"] = "0";
        res["pid"] = to_string(fd);
        res["username"] = req["username"];
        res["cmd"] = "login";
        send_respond(fd,res);

    }   else if (req["cmd"] == "listlobby"){
        /* message type:listlobby <pageno> */
        /* return type: listlobby <list of pair of lobby id and lobbyname> */
        std::unordered_map<string,string> res;
        res["cmd"] = "listlobby";
        int pageno = std::stoi(req["pageno"]);
        auto result = _session_module.activatedList(10,pageno);

        for (auto iter = result.begin() ; iter != result.end(); ++ iter){
            res[std::to_string(iter->first)] = iter->second;
        }
        send_respond(fd,res);
    }   else{
paramter_fail:
        std::unordered_map<string,string> res;
        res["success"] = "0";
        send_respond(fd,res);
    }
}

//TODO
void GameServer::onAcceptConnection(int fd){
    std::cout << "accept connection" << std::endl;
}

std::unordered_map<string,string> req_parse(const string& mess){
    std::vector<std::string> tokens;
    std::istringstream iss(mess);
    std::copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(),
              back_inserter(tokens));

    std::unordered_map<string,string> temp;
    for (auto iter = tokens.begin() ; iter != tokens.end(); iter+=2){
        temp[*iter] = *(iter+1);
    }
    return temp;
};

string res_parse(const std::unordered_map<string,string>& map){
    string result = "";

    for (auto iter = map.begin() ; iter != map.end(); ++iter){
        if (iter == map.begin())
            result += iter->first;
        else
            result += " " + iter->first;
        result+= " " + iter->second;
    }
    return result;
};

int GameServer::send_respond(int fd, const std::unordered_map<string,string>& map){
    auto res_str = res_parse(map);
    TCPServer::packet_t respond{res_str.length(), res_str.c_str()};
    log.LOG("### respond ### "+res_str);
    sendPacket(fd, &respond);
}
