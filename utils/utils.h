//
// Created by alan on 10/20/17.
//

#ifndef GAMESERVER_UTILS_H
#define GAMESERVER_UTILS_H
#include <random>
#include <utility>
#include <vector>
#include <chrono>

class Util{
public:
    int randomRange(int begin, int end);
    std::pair<int,int> randomPairInRange(int begin,int end);

};


#endif //GAMESERVER_UTILS_H
