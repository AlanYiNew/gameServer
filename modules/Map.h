//
// Created by alan on 10/20/17.
//

#ifndef GAMESERVER_MAP_H
#define GAMESERVER_MAP_H
#include "../utils/utils.h"
#include <utility>
#include <vector>

class MapModule{
private:
    int _num = 4;
    std::vector<int> _maps = {4};

public:
    int getRandomMap();
    std::pair<int,int> randomSpawns(int mapid);
    int randomSpawn(int mapid);
};

#endif //GAMESERVER_MAP_H
