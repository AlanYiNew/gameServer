#include <iostream>
#include "GameServer.h"
#include <exception>


/*
struct pos_t{
    float x;
    float y;
};
*/



int main(int argc, char * argv[]){
    try {
        GameServer gs(8666, 8655);//Right now I configure aws to open 8666 for udp 855 for TCP
        gs.starts();
        return 0;
    }   catch(std::exception& ex){
        std::cout << ex.what() << std::endl;
    }
}


