#include "Modules.h"
#include <iostream>



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

int SessionModule::start(int sid){
    _session_bucket[sid]._starts = true;
}

int SessionModule::end(int sid){
    _session_bucket[sid]._starts = false;
}

int SessionModule::create(string lobbyname){
    if (!_available) return -1;
    _session_bucket[_nextfree]._occupied = 0;
    _session_bucket[_nextfree]._players[0] = 0;
    _session_bucket[_nextfree]._players[1] = 0;
    _session_bucket[_nextfree]._lobbyname = lobbyname;
    _available--;
    _activated_session[_nextfree] = &_session_bucket[_nextfree];
    int result = _nextfree;

    do  {
        _nextfree= (_nextfree+1)%MAX_SESSION;
    }   while (_available && !isEmpty(_nextfree));
    return result;
}

int PlayerModule::update(int fd, void * data, int length){
    auto iter =_map.find(fd);
    if (iter != _map.end()) {
        memcpy(iter->second._data.get(),data, length);
    }

	return 0;
}

bool SessionModule::enter(int sid,int fd){
    if (!isFull(sid)) {
        for (int i = 0;i < 2; ++i) {
            if (_session_bucket[sid]._players[i] == 0) {
                _session_bucket[sid]._players[i] = fd;
                break;
            }
        }
        _session_bucket[sid]._occupied++;
        return true;
    }   else{
        return false;
    }
}

int SessionModule::exit(int sid, int fd){
    if (validSid(sid)) {
        _session_bucket[sid]._starts = false;
        _session_bucket[sid]._occupied--;
        for (int i = 0; i < 2 ;++i)
            if (_session_bucket[sid]._players[i] == fd)
                _session_bucket[sid]._players[i] = 0;

        if (isEmpty(sid))
            _activated_session.erase(sid);
        return sid;
    }   else
        return -1;
}

map<int,string> SessionModule::activatedList(int pagesize, int pageno){
    auto iter = _activated_session.begin();
    map<int,string>result;
    for (int i = 0; i < pagesize*(pageno-1) && iter != _activated_session.end(); ) {
        if (!isStarted(i)) ++i;
        ++iter;
    }

    for (int i = 0; i < pagesize && iter != _activated_session.end() ;++iter){
        if (!iter->second->_starts)
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


