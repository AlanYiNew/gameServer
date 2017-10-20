//
// Created by alan on 10/19/17.
//

#ifndef HELLLOWORLD_GS_LOG_H
#define HELLLOWORLD_GS_LOG_H
#include <ctime>
#include <string>
#include <cstring>
#include <iostream>

class GS_LOG{
private:
    std::ostream & os;
public:
    GS_LOG(std::ostream&);
    int LOG(const std::string&);

};

#endif //HELLLOWORLD_GS_LOG_H


