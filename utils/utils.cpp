//
// Created by alan on 10/20/17.
//
#include "utils.h"
int Util::randomRange(int begin, int end){
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(begin,end);
    return distribution(generator);
}

std::pair<int,int> Util::randomPairInRange(int begin,int end){
    std::vector<int> vec;
    for (int i = begin; i <= end;++i){
        vec.push_back(i);
    }

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0,end-begin);
    int val1 = distribution(generator);
    std::swap(vec[val1],vec[vec.size()-1]);
    distribution = std::uniform_int_distribution<int>(0,end-begin-1);
    int val2 = distribution(generator);

    return {vec[vec.size()-1],vec[val2]};
};