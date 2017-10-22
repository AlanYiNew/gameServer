//
// Created by alan on 10/21/17.
//

#ifndef GAMESERVER_PLAYER_H
#define GAMESERVER_PLAYER_H
#include "Modules.h"

class PlayerModule{
public:
    int record(int fd, string username);

    int clear(int fd);



    Player * getPlayer(int fd);

    bool confirm(int fd,int wid, int cid);

    inline bool validPid(int fd);

private:
    map<int,Player> _map;
    size_t _player_buffer_size = 128;
};



inline bool PlayerModule::validPid(int fd){
    return fd > 2;
}
#endif //GAMESERVER_PLAYER_H
