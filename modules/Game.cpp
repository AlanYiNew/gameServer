//
// Created by alan on 10/21/17.
//
#include "Game.h"
#include <iostream>


bool GameModule::validGame(int sid){
    return _map.find(sid) != _map.end();
}

int GameModule::newGame(size_t bufsize,int f1, int f2, int lid){

    _map.emplace(_next_free,Game(bufsize,f1,f2,lid));
    int result = _next_free;
    while (_map.size() != MAX_SESSION && _map.find(_next_free) != _map.end()){
        _next_free = (_next_free+1)%MAX_SESSION;
    }
    return result;
}

int GameModule::getLid(int sid) {
    return _map.find(sid)->second._lid;
}

bool GameModule::count(int sid,int pid){
    //around 5 secs
    if (_map.find(sid)->second._count[pid]++ == 500){
        _map.find(sid)->second._count[pid] = 0;
        return true;
    }
    return false;
}

bool GameModule::lost_count(int sid,int pid){
    return ++_map.find(sid)->second._lost_count[pid] >= 2;
}

bool GameModule::reset_lcount(int sid,int pid){
    //disconnected after lost for three times
    _map.find(sid)->second._lost_count[pid] = 0;
}


std::unordered_map<int,int> GameModule::dead(int sid,int pid){
    auto &g = _map.find(sid)->second;
    int opponent_fd = getOpponent(sid,pid);
    g._score[opponent_fd]++;

    return g._score;

}

int GameModule::updateGame(int sid, int fd, void * data, int length){
    auto iter = _map.find(sid)->second._data.find(fd);
    memcpy(iter->second.get(), data, length);
    return 0;
}

void* GameModule::opponentData(int sid, int fd){
    auto &g = _map.find(sid)->second;
    for (auto iter = g._data.begin(); iter != g._data.end(); ++iter){
        if (iter->first != fd){
            return iter->second.get();
        }
    }
    return 0;
}

int GameModule::getOpponent(int sid, int fd){
    auto &g = _map.find(sid)->second;
    for (auto iter = g._data.begin(); iter != g._data.end(); ++iter){
        if (iter->first != fd){
            return iter->first;
        }
    }
    return 0;
}

void GameModule::clear(int sid){
    if (validGame(sid)){
        _map.erase(sid);
    }
}
