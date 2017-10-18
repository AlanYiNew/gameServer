#ifndef __MODULES_H_
#define __MODULES_H_
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <utility>
using namespace std;

#define MAX_SESSION 8
struct Player{
	void * _data;
	int _len;
	int _fd;//file descriptor
    int _index;
	int _score;
    int _wid; //Weapon id
    int _cid; //character id
	string _username;//TODO name is here
	Player(int fd):_data(nullptr),_len(0),_username("Noob"),_fd(fd){};
	Player(string u,int fd):_data(nullptr),_len(0),_username(u),_fd(fd){};
};

struct chunk{
	int sid;
	int pid;
};

struct session{
	Player* _players[2];
    unsigned int _confirmed;
    unsigned int _starts;
	int _occupied;
	string _lobbyname;
    session():_occupied(0),_confirmed(0),_starts(0),_lobbyname(""){_players[0]=nullptr;_players[1]=nullptr;};
};

class PlayerModule{
public:	
	int record(int fd, string username);

	int clear(int fd);

	Player * getPlayer(int fd);

private:
	map<int,Player> _map;
};

class SessionModule{
public:
	int create(string lobbyname, Player* p);

	int enter(int sid,Player* p);

	int exit(int sid, int index);

    vector<pair<int,string>> activatedList(int pagesize, int pageno);

	unsigned int confirm(int sid, int index);

	unsigned int startGame(int sid, int index);

	void* update(int sid, int index, void* data, int length);

	string getLobbyName(int sid);

    const Player* getPlayer(int sid, int index);

    unsigned int confirmState(int sid);

private:
	session _session_bucket[MAX_SESSION];
    int _nextfree = 0;
    int _available = MAX_SESSION;
    std::map<int , session*> _activated_session;
    bool validSid(int sid);
};

#endif