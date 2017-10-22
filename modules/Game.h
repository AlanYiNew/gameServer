//
// Created by alan on 10/21/17.
//

#ifndef GAMESERVER_GAME_H
#define GAMESERVER_GAME_H
#include <unordered_map>
#include "Modules.h"



class GameModule{
private:
    struct Game{
        std::unordered_map<int,std::unique_ptr<char[]>> _data;
        std::unordered_map<int,int> _score;
        std::unordered_map<int,int> _lost_count;
        int _lid;
        std::unordered_map<int,int> _count;

        Game(size_t buf_size,int fd1, int fd2, int lid){
            _data.emplace(fd1,std::make_unique<char[]>(buf_size));
            _data.emplace(fd2,std::make_unique<char[]>(buf_size));
            _score[fd1] = 0;
            _score[fd2] = 0;
            _lid = lid;
            _count[fd1] = 0;
            _count[fd2] = 0;
            _lost_count[fd1] = 0;
            _lost_count[fd2] = 0;
        };

    };
    std::unordered_map<int,Game> _map;
    int _next_free = 0;

public:
    int newGame(size_t bufsize,int f1, int f2, int lid);
    int updateGame(int sid, int fd, void * data, int length);
    void* opponentData(int sid, int fd);
    int getLid(int);
    bool validGame(int sid);
    void clear(int sid);
    int getOpponent(int sid, int fd);
    bool count(int sid,int fd);
    bool lost_count(int sid, int pid);
    bool reset_lcount(int sid, int pid);

    std::unordered_map<int,int> dead(int sid,int pid);
};

#endif //GAMESERVER_GAME_H
