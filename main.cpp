#include <iostream>
#include "GameServer.h"



/*
struct pos_t{
    float x;
    float y;
};
*/



int main(int argc, char * argv[]){
    try {
        const string sc_path = "gameServerProto/proto/client.txt";
        GameServer gs(8666, 8655,sc_path);//Right now I configure aws to open 8666 for udp 855 for TCP
        gs.starts();
        return 0;
    }   catch(exception& ex){
        cout << ex.what() << endl;
    }
}


