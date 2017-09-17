#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<sys/epoll.h>
#include<set>
#include<vector>
//
// Created by alan on 9/17/17.
//

#ifndef __TCPSERVER_H_
#define __TCPSERVER_H_

class TCPServer{
    private : 
        int port;
        sockaddr_in serv_addr;
        epoll_event events[65535];
        int event_num;
        std::vector<int> clientfds;

public :
        /** TCPServer class constructor **/
        TCPServer(int port,int event_num);
        ~TCPServer();

        /** Initialize a TCPServer and prepare to set up listening **/
        int init();

        /** starts the TCPServer **/
        int starts();
 
};

#endif //__TCPSERVER_H_