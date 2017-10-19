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
    int _session;
	int _score;
    int _wid; //Weapon id
    int _cid; //character id
	string _username;
    bool _confirmed;
	Player(int fd):_data(nullptr),_len(0),_username("Noob"),_fd(fd),_confirmed(0),_wid(0),_cid(0),_session(-1){};
	Player(string u,int fd):_data(nullptr),_len(0),_username(u),_fd(fd),_confirmed(0),_wid(0),_cid(0),_session(-1){};
};

struct chunk{
	int sid;
	int pid;
};

struct session{
	int _players[2];
    bool _starts;
	int _occupied;
	string _lobbyname;
    session():_occupied(0),_starts(false),_lobbyname(""){_players[0]=0;_players[1]=0;};
};

class PlayerModule{
public:	
	int record(int fd, string username);

	int clear(int fd);

    int update(int fd, void * data, int length);

	Player * getPlayer(int fd);

    bool confirm(int fd,int wid, int cid);

private:
	map<int,Player> _map;
};

class SessionModule{
public:
	int create(string lobbyname, int fd);

	int enter(int sid,int fd);

	int exit(int sid, int index);

    map<int,string> activatedList(int pagesize, int pageno);

    vector<int> getPlayerPids(int sid);

    int start(int sid);

    int end(int sid);

	const string& getLobbyName(int sid);

    const int getOpponent(int sid, int fd);

    bool validSid(int sid);

    int clear(int sid);

private:
	std::array<session,MAX_SESSION> _session_bucket;
    int _nextfree = 0;
    int _available = MAX_SESSION;
    std::map<int , session*> _activated_session;

};

#endif