#ifndef __MODULES_H_
#define __MODULES_H_
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <utility>



using namespace std;

#define MAX_SESSION 256
struct Player{
	void * _data;
	int _len;
	int _fd;//file descriptor
    int _index;
	int _score;
    int _wid; //Weapon id
    int _cid; //character id
	string _username;
    bool _confirmed;
	Player(int fd):_data(nullptr),_len(0),_username("Noob"),_fd(fd),_confirmed(0){};
	Player(string u,int fd):_data(nullptr),_len(0),_username(u),_fd(fd),_confirmed(0){};
};

struct chunk{
	int sid;
	int pid;
};

struct session{
	int _players[2];
    unsigned int _starts;
	int _occupied;
	string _lobbyname;
    session():_occupied(0),_starts(0),_lobbyname(""){_players[0]=0;_players[1]=0;};
};

class PlayerModule{
public:	
	int record(int fd, string username);

	int clear(int fd);

    int update(int fd, void * data, int length);

	Player * getPlayer(int fd);

    bool confirm(int fd);

private:
	map<int,Player> _map;
};

class SessionModule{
public:
	int create(string lobbyname, int fd);

	int enter(int sid,int fd);

	int exit(int sid, int index);

    vector<pair<int,string>> activatedList(int pagesize, int pageno);

	int startGame(int sid, int index);

	string getLobbyName(int sid);

    const int getOpponent(int sid, int fd);

private:
	std::array<session,MAX_SESSION> _session_bucket;
    int _nextfree = 0;
    int _available = MAX_SESSION;
    std::map<int , session*> _activated_session;
    bool validSid(int sid);
};

#endif