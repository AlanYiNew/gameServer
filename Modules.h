#include <map>
#include <string>
using namespace std;

#define MAX_SESSION 8
#define MAX_PLAYERS 2

struct Player{
	void * data;
	int len;
	int fd;//file descriptor
	bool confirmed;
	bool starts;
	int score;
	string username;//TODO name is here
	Player():data(nullptr),len(0),confirmed(0),starts(0),username("Noob"){};
	Player(string u):data(nullptr),len(0),confirmed(0),starts(0),username(u){};
};

struct chunk{
	int sid;
	int pid;
};

struct session{
	Player* players[2];
	int occupied;
	string lobbyname; 
};

class PlayerModule{
public:	
	int record(int fd, string username);

	int clear(fd);

	Player * getPlayer(int fd);

private:
	map<fd,Player> _map;
}

class SessionModule{
public:
	int create(string lobbyname, Player* p);

	int enter(int sid,Player* p);

	int exit(int sid, int pid);

	int activated_list(int pageno);

	int confirm(int sid, int pid);

	int start_game(int sid, int pid);

	int update(int sid, int pid, void* data, int length);
private:
	session _session_bucket[MAX_SESSION];
}	
