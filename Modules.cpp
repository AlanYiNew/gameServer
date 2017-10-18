#include "Modules.h"

class PlayerModule{
	public: 
		int record(int fd, string username);

		int clear(fd);

		Player* getPlayer(int fd);
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
	
		int update(int sid, int pid, void * data, int length);
	
	private:
		session _session_bucket[MAX_SESSION];
}

int PlayerModule::record(int fd, string u){
	_map.emplace({fd,Player(u)});
}

int PlayerModule::clear(int fd){
	if (_map[fd].data != nullptr){
		free(_map[fd].data);
	}	
};

int SessionModule::create(string lobbyname, Player* p){
		

}
SessionModule::update(int sid, int pid, void * data, int length){
	memcpy(session_bucket[recv.sid].players[recv.pid]->data,data, length);	
}


