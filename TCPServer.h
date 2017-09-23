#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include<sys/epoll.h>
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
        int epollfd;
        int pending_num;

        int epoll_add(int fd);
        int epoll_del(int fd);

        struct _header{
            int len;
        };

public :
        /** TCPServer class constructor **/
        TCPServer(int port);
        ~TCPServer();

        /** Initialize a TCPServer and prepare to set up listening **/
        int init();

        /** starts the TCPServer **/
        int starts();

        /** pure event driven virtual function **/
        virtual void onShutDownConnection(int fd)=0;
        virtual void onRead(int fd, char *, int readsize)=0;
        virtual void onAcceptConnection(int fd)= 0;
        int send(int fd,char * message,int read_size);
 
};

#endif //__TCPSERVER_H_