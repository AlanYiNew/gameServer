//
// Created by alan on 10/21/17.
//

#ifndef GAMESERVER_SESSION_H
#define GAMESERVER_SESSION_H
#include <string>
#include "Modules.h"
class SessionModule{
public:
    int create(string lobbyname);

    bool enter(int sid,int fd);

    bool exit(int sid, int index);

    map<int,string> activatedList(int pagesize, int pageno);

    vector<int> getPlayerPids(int sid);

    const string& getLobbyName(int sid);

    const int getOpponent(int sid, int fd);


    int clear(int sid);

    inline bool isFull(int sid);

    inline bool isEmpty(int sid);

    inline bool validSid(int sid);

private:
    std::array<session,MAX_SESSION> _session_bucket;
    int _nextfree = 0;
    int _available = MAX_SESSION;
    std::map<int , session*> _activated_session;


};

inline bool SessionModule::isFull(int sid){
    return sid >= 0 && sid < MAX_SESSION && _session_bucket[sid]._occupied == 2;
}

inline bool SessionModule::isEmpty(int sid){
    return sid >= 0 && sid < MAX_SESSION && _session_bucket[sid]._occupied == 0;
}

inline bool SessionModule::validSid(int sid){
    return sid >= 0 && sid < MAX_SESSION;
}
#endif //GAMESERVER_SESSION_H
