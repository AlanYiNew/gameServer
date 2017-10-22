#ifndef __MODULES_H_
#define __MODULES_H_
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <utility>
#include <memory>
#include <unordered_map>

using namespace std;

enum GameStatus{
    INLOBBY,INROOM,INGAME
};

#define MAX_SESSION 256
struct Player{
	int _len;
	int _fd;//file descriptor
    GameStatus _status = INLOBBY;
    int _session;
    int _wid; //Weapon id
    int _cid; //character id
	string _username;
    bool _confirmed;
	Player(int fd,size_t buffer_size):_len(0),_username("Noob"),_fd(fd),_confirmed(0),_wid(0),_cid(0),_session(-1){
    };
	Player(string u,int fd,size_t buffer_size):_len(0),_username(u),_fd(fd),_confirmed(0),_wid(0),_cid(0),_session(-1){
    };

    void reset(){
        _wid = 0;
        _cid = 0;
        _confirmed = 0;
    }

};





struct chunk{
	int sid;
	int pid;
};

struct session{
	int _players[2];
	//TODO isolate data with Player object


	int _occupied;
	string _lobbyname;
    session():_occupied(0),_lobbyname(""){
		_players[0]=0;_players[1]=0;
	};
};


#endif