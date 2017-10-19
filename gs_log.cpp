//
// Created by alan on 10/19/17.
//
#include "gs_log.h"


int GS_LOG::LOG(const std::string& str){
    time_t now = time(0);
    char* dt = ctime(&now);
    dt[strlen(dt) - 1] = '\0';
    os << "[" << dt << "]: "<< str << std::endl;
    return 0;
}

GS_LOG::GS_LOG(std::ostream& os_arg):os(os_arg){}