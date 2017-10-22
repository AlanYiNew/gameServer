//
// Created by alan on 10/21/17.
//
#include "Session.h"

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

bool SessionModule::exit(int sid, int fd){
    if (validSid(sid)) {
        _session_bucket[sid]._occupied--;
        for (int i = 0; i < 2 ;++i)
            if (_session_bucket[sid]._players[i] == fd)
                _session_bucket[sid]._players[i] = 0;

        if (isEmpty(sid))
            clear(sid);
        return true;
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
    if (_activated_session.find(sid) != _activated_session.end())
        _activated_session.erase(sid);
}


map<int,string> SessionModule::activatedList(int pagesize, int pageno){
    auto iter = _activated_session.begin();
    map<int,string>result;
    for (int i = 0; i < pagesize*(pageno-1) && iter != _activated_session.end(); ) {
        ++iter;
    }

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
