//
// Created by alan on 10/21/17.
//

#include "Modules.h"
#include "Player.h"




int PlayerModule::record(int fd, string u){
    if (_map.find(fd) == _map.end())
        _map.emplace(fd,Player(u,fd,_player_buffer_size));
    return 0;
}

int PlayerModule::clear(int fd){
    auto iter = _map.find(fd);
    if (iter != _map.end()) {
        _map.erase(iter);
        return 0;
    }
    return -1;
};

Player * PlayerModule::getPlayer(int fd){
    if (_map.find(fd) != _map.end())
        return &_map.find(fd)->second;
    else
        return nullptr;
}



bool PlayerModule::confirm(int fd, int wid, int cid){
    auto iter = _map.find(fd);
    if (iter != _map.end()) {
        iter->second._confirmed = !iter->second._confirmed;
        iter->second._cid = cid;
        iter->second._wid = wid;
        return iter->second._confirmed;
    }   else
        return false;
}

