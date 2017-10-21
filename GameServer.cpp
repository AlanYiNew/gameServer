//
// Created by alan on 9/23/17.
//

#include "GameServer.h"


using namespace std;


#define SERVER_DEBUG 1

std::unordered_map<string, string> req_parse(const string &mess);

string res_parse(const std::unordered_map<string, string> &map);

string res_parse(const std::map<string, string> &map);

int udp_callback(void *userptr, const UDPServer::message_t &message, UDPServer::message_t &message_out) {
    GameServer *server = reinterpret_cast<GameServer *>(userptr);
    chunk recv = *reinterpret_cast<chunk *>(message.content);

    if (server->_game_module.validGame(recv.sid)) {
        bool active = true;
        int opponent_fd;
        if (server->_game_module.count(recv.sid,recv.pid)){
            //heart beat message
            opponent_fd = server->_game_module.getOpponent(recv.sid,recv.pid);
            active = server->is_alive(opponent_fd);
        }

        if (active) {
            server->_game_module.updateGame(recv.sid, recv.pid, message.content, message.len);
            message_out.content = server->_game_module.opponentData(recv.sid, recv.pid);;
            message_out.len = message.len;
        }   else{
            message_out.len = 0;
            if (server->_game_module.lost_count(recv.sid,opponent_fd))
                server->userForceQuitHandler(opponent_fd,true);
        }

    }   else{
        message_out.len = 0;
    }
#if SERVER_DEBUG
    //server->log.LOG("recving " + std::to_string(message.len) +" from " +  std::to_string(recv.pid));
#endif
    return 0;
}

void GameServer::starts() {
    UDPServer us(_udp_port, _buf_size);
    us.init();//buffer size
    us.register_cb(udp_callback, this);
    std::thread thread([&us]() {
        us.starts();
    });

    TCPServer::init();

    TCPServer::starts();
}

GameServer::GameServer(int udp_port, int tcp_port,const string& sc_path) :
        TCPServer(tcp_port),
        _tcp_port(tcp_port),
        _udp_port(udp_port),
        log(std::cout),
        _sanity_check(sc_path) {

};

void GameServer::onShutDownConnection(int fd) {
    const Player *p = _player_module.getPlayer(fd);
    std::cout << "shutDownConnection" << std::endl;
    if (p!= nullptr) {
        log.LOG("DEBUG===Calling FORCEQUITHANLDER");
        userForceQuitHandler(fd, true);
    }
}

void GameServer::onRead(int fd,const char *mess, int readsize) {

    std::string request_mess(mess, readsize);

#if SERVER_DEBUG
    log.LOG("### command ### " + request_mess);
#endif
    auto req = req_parse(request_mess);
    if (req["cmd"] == "create") {
        /*message type: create <lobbyname>*/
        /*return type: create <lobbyname> <pid/fd>*/
        int sid = _session_module.create(req["lobbyname"]);
        Player *p = _player_module.getPlayer(fd);
        std::unordered_map<string, string> res;
        res["cmd"] = "create";
        res["lobbyname"] = req["lobbyname"];
        res["sid"] = to_string(sid);
        res["pid"] = to_string(fd);
        if (_session_module.validSid(sid)) {
            _session_module.enter(sid, fd);
            p->_session = sid;
            res["success"] = "0";

        } else {
            res["success"] = "-1";
        }

        send_respond(fd, res);

    } else if (req["cmd"] == "enter") {

        int sid = std::stoi(req["sid"]);
        Player *entered_player = _player_module.getPlayer(fd);

        std::unordered_map<string, string> res;

        if (_session_module.enter(sid, fd)) {
            const string &lobbyname = _session_module.getLobbyName(sid);
            const int host_player_fd = _session_module.getOpponent(sid, fd);
            const Player *host_player = _player_module.getPlayer(host_player_fd);
            entered_player->_session = sid;
            res["cmd"] = "enter";
            res["sid"] = std::to_string(sid);
            res["lobbyname"] = lobbyname;
            res["hostpid"] = to_string(host_player_fd);
            res["hostname"] = host_player->_username;
            res["hostcid"] = to_string(host_player->_cid);
            res["hostwid"] = to_string(host_player->_wid);
            res["enterpid"] = to_string(fd);
            res["entername"] = entered_player->_username;
            res["entercid"] = to_string(entered_player->_cid);
            res["enterwid"] = to_string(entered_player->_wid);
            res["success"] = "0";


            send_respond(fd, res);
            send_respond(host_player_fd, res);
        } else {

            res["success"] = "-1";
            send_respond(fd, res);
        }

    } else if (req["cmd"] == "exit") {
        /*message type: exit <sid> */
        /*return type: exitted <result>*/

        int sid = std::stoi(req["sid"]);
        int opponenet_fd = _session_module.getOpponent(sid, fd);

        bool result = _session_module.exit(sid, fd);
        std::unordered_map<string, string> res;

        Player *p = _player_module.getPlayer(fd);
        res["pid"] = to_string(fd);
        res["cmd"] = "exit";
        if (result) {
            res["success"] = "0";
            p->_session = -1;
        }   else {
            res["success"] = "-1";
        }

        send_respond(fd, res);


        if (_player_module.validPid(opponenet_fd))
            send_respond(opponenet_fd, res);

    } else if (req["cmd"] == "confirm") {
        /*message type: confirm <pid/fd> <sid> <cid> <wid>*/
        /*return type: confirmed <confirmer's pid/fd> <ready state> <cid> <wid> <o's cid> <p's wid>*/

        int sid = stoi(req["sid"]);
        int cid = stoi(req["cid"]);
        int wid = stoi(req["wid"]);
        unordered_map<string, string> res;
        if (_session_module.validSid(sid)) {
            const int opponent_fd = _session_module.getOpponent(sid, fd);
            bool ready_state = _player_module.confirm(fd, wid, cid);
            Player *p = _player_module.getPlayer(fd);
            Player *opponent = _player_module.getPlayer(opponent_fd);

            res["cmd"] = "confirm";
            res["pid"] = to_string(fd);
            res["readystate"] = to_string(ready_state);

            res["success"] = "0";

            if (_player_module.validPid(opponent_fd)) {
                send_respond(opponent_fd, res);
            }
            send_respond(fd, res);

            if (p->_confirmed && opponent != nullptr && opponent->_confirmed) {
                Player * opponent = _player_module.getPlayer(opponent_fd);
                res["cmd"] = "gamestart";
                int lid = _map_module.getRandomMap();
                res["lid"] = to_string(lid);
                auto pair = _map_module.randomSpawns(lid);
                res["playerspawnpoint"] = to_string(pair.first);
                res["opponentspawnpoint"] = to_string(pair.second);
                res["playercid"] = to_string(wid);
                res["playerwid"] = to_string(cid);
                res["opponentcid"] = to_string(opponent->_cid);
                res["opponentwid"] = to_string(opponent->_wid);
                res["success"] = "0";
                _session_module.clear(sid);
                _game_module.newGame(sid,_buf_size,fd,opponent_fd,lid);
                send_respond(fd, res);
                send_respond(opponent_fd, res);
            }
        } else {
            res["cmd"] = "confirm";
            res["success"] = "-1";
            send_respond(fd, res);
        }
    } else if (req["cmd"] == "dead") {
        /*message type: dead <sid>*/
        /*return type: score <score> or win or lose*/

        std::unordered_map<string, string> res;
        Player *player = _player_module.getPlayer(fd);
        int sid = player->_session;
        int opponent_fd = _session_module.getOpponent(sid, fd);
        Player *opponent = _player_module.getPlayer(opponent_fd);

        auto result = _game_module.dead(sid,fd);
        res["success"] = "0";
        res["cmd"] = "score";
        res["score"] = to_string(result[opponent_fd]);
        send_respond(opponent_fd, res);

        res["cmd"] = "dead";
        res["playerspawnpoint"] = std::to_string(_map_module.randomSpawn(_session_module.getLid(sid)));
        send_respond(fd,res);

        if (result.size()!=0) {
            std::unordered_map<string, string> res2;
            _session_module.clear(sid);
            res2["opponentscore"] = to_string(result[opponent_fd]);
            res2["playerscore"] = to_string(result[fd]);
            res2["cmd"] = "gameover";
            res2["success"] = "0";
            send_respond(fd,res2);
            send_respond(opponent_fd,res2);
            opponent->reset();
            player->reset();
        }

    } else if (req["cmd"] == "login") {
        /* message type:login <username> */
        /* return type:login <username> <fd/pid> */

        _player_module.record(fd, req["username"]);
        std::unordered_map<string, string> res;

        res["success"] = "0";
        res["pid"] = to_string(fd);
        res["username"] = req["username"];
        res["cmd"] = "login";
        send_respond(fd, res);

    } else if (req["cmd"] == "listlobby") {
        /* message type:listlobby <pageno> */
        /* return type: listlobby <list of pair of lobby id and lobbyname> */

        int pageno = std::stoi(req["pageno"]);

        int count = 0;
        std::map<int, string> result;
        std::map<int, string> prev;
        while (result.size() < 10) {
            result = _session_module.activatedList(10, pageno);
            if (result.size() == prev.size()) break;
            for (auto iter = result.begin(); iter != result.end(); ++iter) {
                if (prev.find(iter->first) == prev.end()) {
                    bool active = false;
                    auto pidlist = _session_module.getPlayerPids(iter->first);
                    log.LOG("DEBUG===pidlistsize "+to_string(pidlist.size()));
                    for (int fd:pidlist) {
                        active = is_alive(fd);
                        if (active) break;
                    }

                    if (!active) {
                        _session_module.clear(iter->first);
                        result.erase(iter->first);
                    }
                }
            }
            prev = result;
            count++;
        }

        send_respond(fd, result);
    }   else{
        std::unordered_map<string, string> res;
        res["success"] = "-1";
        res["hint"] = "invalid arguments";
        send_respond(fd, res);
    }
}

//TODO
void GameServer::onAcceptConnection(int fd) {
    std::cout << "accept connection" << std::endl;
    std::unordered_map<string, string> res;
    res["success"] = "0";
    res["cmd"] = "connected";
    send_respond(fd, res);

    Player *p = _player_module.getPlayer(fd);
    if (p != nullptr) {
        userForceQuitHandler(fd,false);
    }
}

std::unordered_map<string, string> req_parse(const string &mess) {
    std::vector<std::string> tokens;
    std::istringstream iss(mess);
    std::copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(),
              back_inserter(tokens));

    std::unordered_map<string, string> temp;
    for (auto iter = tokens.begin(); iter != tokens.end(); iter += 2) {
        temp[*iter] = *(iter + 1);
    }
    return temp;
};

string res_parse(const std::unordered_map<string, string> &map) {
    string result = "";

    for (auto iter = map.begin(); iter != map.end(); ++iter) {
        if (iter == map.begin())
            result += iter->first;
        else
            result += " " + iter->first;
        result += " " + iter->second;
    }
    return result;
};

//specialize for list
string res_parse(const std::map<int, std::string> &map) {
    string result = "";

    for (auto iter = map.begin(); iter != map.end(); ++iter) {
        if (iter == map.begin())
            result += to_string(iter->first);
        else
            result += " " + to_string(iter->first);
        result += " " + iter->second;
    }
    if (result.size() == 0)
        result += "cmd listlobby";
    else
        result += " cmd listlobby";
    return result;
};

void GameServer::userForceQuitHandler(int fd,bool send){
    Player *p = _player_module.getPlayer(fd);

    if (_game_module.validGame(p->_session) ||
        _session_module.exit(p->_session,fd)){
        log.LOG("DEBUG===INSIDE if statement FORCEQUITHANLDER");
        int oppnent_fd;
        if (_game_module.validGame(p->_session)){
            oppnent_fd = _game_module.getOpponent(p->_session,fd);
        }   else{
            oppnent_fd = _session_module.getOpponent(p->_session,fd);
        }

        if (send) {
            std::unordered_map<string, string> res;
            res["cmd"] = res["exit"];
            res["pid"] = to_string(fd);
            res["success"] = "0";
            send_respond(oppnent_fd, res);
        }
        _game_module.clear(p->_session);
    }

    _player_module.clear(fd);
}

int GameServer::send_respond(int fd, const std::unordered_map<string, string> &map) {
    assert(!_sanity_check.isValid(map) && "invalid respond");
    auto res_str = res_parse(map);
    TCPServer::packet_t respond{res_str.length(), res_str.c_str()};
    int len = sendPacket(fd, &respond);
    log.LOG("### respond ### " + res_str + " " + to_string(len));
    return len;
}

int GameServer::send_respond(int fd, const std::map<int, string> &map) {
    auto res_str = res_parse(map);
    TCPServer::packet_t respond{res_str.length(), res_str.c_str()};
    int len = sendPacket(fd, &respond);
    log.LOG("### respond ### " + res_str + " " + to_string(len));
    return len;
}

bool GameServer::is_alive(int fd) {
    std::unordered_map<string, string> res;
    res["cmd"] = "isalive";
    return send_respond(fd, res) > 0;
}

SCChecker::SCChecker(const string& str){
    try {
        std::ifstream ifs(str);
        string line;
        assert(!ifs.fail() && "Fail to open sanity check file");

        while (std::getline(ifs, line)) {

            std::vector<std::string> tokens;
            std::istringstream iss(line);
            std::copy(std::istream_iterator<std::string>(iss),
                      std::istream_iterator<std::string>(),
                      back_inserter(tokens));
            sc[tokens[0]] = unordered_set<string>();
            for (auto iter = tokens.begin(); iter != tokens.end(); ++iter) {
                if (iter != tokens.begin()) {
                    sc[tokens[0]].insert(*iter);
                }
            }
        }
    } catch (exception &ex) {
        std::cout << ex.what() << std::endl;
    }
}

bool SCChecker::isValid(const std::unordered_map<string,string> &req){
    auto const_iter = req.find("cmd");
    if (const_iter != req.end()) {

        auto iter = sc.find(const_iter->first);
        if (iter != sc.end()) {
            for (const string &key: iter->second) {
                if (req.find(key) == req.end()) {
                    std::cout << "lack " << key << std::endl;
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }   else{
        return false;
    }
}

