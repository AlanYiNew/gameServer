#ifndef __MODULES_H_
#define __MODULES_H_
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <utility>
#include <memory>



using namespace std;

#define MAX_SESSION 256
struct Player{
	std::unique_ptr<char[]> _data;
	int _len;
	int _fd;//file descriptor
    int _session;
	int _score;
    int _wid; //Weapon id
    int _cid; //character id
	string _username;
    bool _confirmed;
	Player(int fd,size_t buffer_size):_len(0),_username("Noob"),_fd(fd),_confirmed(0),_wid(0),_cid(0),_session(-1){
        _data = make_unique<char[]>(buffer_size);

    };
	Player(string u,int fd,size_t buffer_size):_len(0),_username(u),_fd(fd),_confirmed(0),_wid(0),_cid(0),_session(-1){
        _data = make_unique<char[]>(buffer_size);
    };

    void reset(){
        _score = 0;
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
    size_t _player_buffer_size = 128;
};

class SessionModule{
public:
	int create(string lobbyname);

	bool enter(int sid,int fd);

	int exit(int sid, int index);

    map<int,string> activatedList(int pagesize, int pageno);

    vector<int> getPlayerPids(int sid);

    int start(int sid);

    int end(int sid);

	const string& getLobbyName(int sid);

    const int getOpponent(int sid, int fd);

    int clear(int sid);

    inline bool isFull(int sid);

    inline bool isEmpty(int sid);

    inline bool isStarted(int sid);

    inline bool validSid(int sid);

private:
	std::array<session,MAX_SESSION> _session_bucket;
    int _nextfree = 0;
    int _available = MAX_SESSION;
    std::map<int , session*> _activated_session;


};

inline bool SessionModule::isFull(int sid){
    return sid >= 0 && sid < MAX_SESSION && _session_bucket[sid]._occupied == 2;
}

inline bool SessionModule::isEmpty(int sid){
    return sid >= 0 && sid < MAX_SESSION && _session_bucket[sid]._occupied == 0;
}

inline bool SessionModule::isStarted(int sid){
    return _session_bucket[sid]._starts;
}

inline bool SessionModule::validSid(int sid){
    return sid >= 0 && sid < MAX_SESSION;
}

#endif