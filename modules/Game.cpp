//
// Created by alan on 10/21/17.
//
#include "Game.h"


bool GameModule::validGame(int sid){
    return _map.find(sid) != _map.end();
}

void GameModule::newGame(size_t bufsize,int f1, int f2, int lid){
    _map.emplace(_next,Game(bufsize,f1,f2,lid));
    if (_map.size() < MAX_SESSION){
        while (_map.find(_next) != _map.end()){
            _next = (_next+1)%MAX_SESSION;
        }
    }
}

int GameModule::getLid(int sid) {
    return _map.find(sid)->second._lid;
}

std::unordered_map<int,int> GameModule::dead(int sid,int pid){
    auto &g = _map.find(sid)->second;
    auto iter = g._score.begin();
    for (; iter != g._score.end(); ++iter){
        if (iter->first != pid){
            iter->second++;
            break;
        }
    }

    if (iter->second >=3){
        return g._score;
    }
    return std::unordered_map<int,int>();
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

int GameModule::getOpponentData(int sid, int fd){
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