#include "Modules.h"



int PlayerModule::record(int fd, string u){
    if (_map.find(fd) != _map.end())
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

int SessionModule::create(string lobbyname, Player* p){
    if (!_available) return -1;
    _session_bucket[_nextfree]._occupied = 1;
    _session_bucket[_nextfree]._players[0] = p;
    _session_bucket[_nextfree]._lobbyname = lobbyname;
    _available--;
    p->_index = 0;
    int result = _nextfree;
    while (_available && _session_bucket[_nextfree]._occupied) {
        _nextfree= (_nextfree+1)%MAX_SESSION;
    }
    return result;
}

void * SessionModule::update(int sid, int index, void * data, int length){
    if (_session_bucket[sid]._players[index]->_data == nullptr) {
        _session_bucket[sid]._players[index]->_data = malloc(length);
    }
	memcpy(_session_bucket[sid]._players[index]->_data,data, length);
	return _session_bucket[sid]._players[index^1]->_data;
}

int SessionModule::enter(int sid,Player* p){
    if (_session_bucket[sid]._occupied == 1) {
        _session_bucket[sid]._players[1] = p;
        _session_bucket[sid]._occupied++;
        p->_index = 1;
        return sid;
    }   else{
        return -1;
    }
}

int SessionModule::confirm(int sid, int index){
    _session_bucket[sid]._confirmed ^= (1 << index);
    return _session_bucket[sid]._confirmed & (1 << (index))?1:0;
}

bool SessionModule::validSid(int sid){
    return sid >= 0 && sid < MAX_SESSION
           && _session_bucket[sid]._occupied;
}

int SessionModule::exit(int sid, int index){
    if (validSid(sid)) {
        _session_bucket[sid]._occupied--;
        _session_bucket[sid]._starts &= ~(1 << index);
        _session_bucket[sid]._players[index] = nullptr;
        _session_bucket[sid]._confirmed &= ~(1 << index);
        return sid;
    }   else
        return -1;
}

vector<pair<int,string>> SessionModule::activatedList(int pagesize, int pageno){
    auto iter = _activated_session.begin();
    for (int i = 0; i < pagesize*(pageno-1) && i < _activated_session.size(); ++i)
        ++iter;

    vector<pair<int,string>> result;
    for (int i = 0; i < pagesize && iter != _activated_session.end() ;++iter){
        result.push_back({iter->first,iter->second->_lobbyname});
    }
    return result;
}

const Player* SessionModule::getPlayer(int sid, int index){
    return _session_bucket[sid]._players[index];
}

int SessionModule::startGame(int sid, int index){
    _session_bucket[sid]._starts |= (1 << index);
    return (_session_bucket[sid]._starts & (1 << (1^index)))?1:0;
}

unsigned int SessionModule::confirmState(int sid){
    return _session_bucket[sid]._confirmed;
}

string SessionModule::getLobbyName(int sid){
    return _session_bucket[sid]._lobbyname;
}