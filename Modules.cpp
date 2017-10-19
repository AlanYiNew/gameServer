#include "Modules.h"
#include <iostream>



int PlayerModule::record(int fd, string u){
    if (_map.find(fd) == _map.end())
	    _map.emplace(fd,Player(u,fd));
    return 0;
}

int PlayerModule::clear(int fd){
    auto iter = _map.find(fd);
    if (iter != _map.end()) {
        if (iter->second._data != nullptr)
            free(iter->second._data);
        return 0;
    }
    return -1;
};

Player * PlayerModule::getPlayer(int fd){
    return &_map.find(fd)->second;
}

int SessionModule::create(string lobbyname, int fd){
    if (!_available) return -1;
    _session_bucket[_nextfree]._occupied = 1;
    _session_bucket[_nextfree]._players[0] = fd;
    _session_bucket[_nextfree]._lobbyname = lobbyname;
    _session_bucket[_nextfree]._players[1] = 0;
    _available--;
    _activated_session[_nextfree] = &_session_bucket[_nextfree];
    int result = _nextfree;
    while (_available && _session_bucket[_nextfree]._occupied) {
        _nextfree= (_nextfree+1)%MAX_SESSION;
    }
    return result;
}

int PlayerModule::update(int fd, void * data, int length){
    auto iter =_map.find(fd);
    if (iter != _map.end()) {
        iter->second._data = malloc(length);
    }
	memcpy(iter->second._data,data, length);
	return 0;
}

int SessionModule::enter(int sid,int fd){
    if (_session_bucket[sid]._occupied == 1) {
        _session_bucket[sid]._players[1] = fd;
        _session_bucket[sid]._occupied++;
        return sid;
    }   else{
        return -1;
    }
}



bool SessionModule::validSid(int sid){
    return sid >= 0 && sid < MAX_SESSION
           && _session_bucket[sid]._occupied;
}

int SessionModule::exit(int sid, int fd){
    if (validSid(sid)) {
        _session_bucket[sid]._occupied--;
        for (int i = 0; i < 2 ;++i)
            if (_session_bucket[sid]._players[i] == fd)
                _session_bucket[sid]._players[i] = 0;

        if (_session_bucket[sid]._occupied == 0)
            _activated_session.erase(sid);
        return sid;
    }   else
        return -1;
}

map<int,string> SessionModule::activatedList(int pagesize, int pageno){
    auto iter = _activated_session.begin();
    for (int i = 0; i < pagesize*(pageno-1) && i < _activated_session.size(); ++i)
        ++iter;

    map<int,string>result;
    for (int i = 0; i < pagesize && iter != _activated_session.end() ;++iter){
        result[iter->first] = iter->second->_lobbyname;
    }
    return result;
}

const int SessionModule::getOpponent(int sid, int fd){
    for (int i = 0; i < 2 ;++i){
        if (_session_bucket[sid]._players[i] != fd) return _session_bucket[sid]._players[i];
    }
}


const string& SessionModule::getLobbyName(int sid){
    return _session_bucket[sid]._lobbyname;
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

vector<int> SessionModule::getPlayerPids(int sid){
    vector<int> result;
    for (int i = 0 ; i < 2; i++){
        if (_session_bucket[sid]._players[i] > 2)
            result.push_back(_session_bucket[sid]._players[i]);
    }
    return result;
}

int SessionModule::clear(int sid){
    _session_bucket[sid]._occupied = 0;
    _session_bucket[sid]._players[0] = 0;
    _session_bucket[sid]._players[1] = 0;
    _session_bucket[sid]._lobbyname = "";
    _activated_session.erase(sid);
}