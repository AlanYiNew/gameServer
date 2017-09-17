#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<sys/epoll.h>
#include<vector>
class Server{
    private : 
        int port;
        sockaddr_in *serv_addr; 
        epoll_event * events;  
        int event_num;
        std::vector<int> clientfds;
    public :
        /** Server class constructor **/
        Server(int port,int event_num);
        ~Server();

        /** Initialize a server and prepare to set up listening **/
        int init();

        /** starts the server **/
        int starts();
 
};
