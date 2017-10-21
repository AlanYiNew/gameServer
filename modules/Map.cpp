//
// Created by alan on 10/20/17.
//
#include "Map.h"


int MapModule::getRandomMap() {
    Util temp;
    return temp.randomRange(0,_maps.size()-1);
}

std::pair<int,int> MapModule::randomSpawns(int mapid) {

    Util temp;

    return temp.randomPairInRange(0,_maps[mapid]-1);
}

int MapModule::randomSpawn(int mapid) {
    Util temp;
    return temp.randomRange(0,_maps[mapid]-1);
}