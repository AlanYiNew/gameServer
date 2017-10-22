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
#define MAXLINE 4096
class TCPServer{
    private : 
        uint16_t port;
        sockaddr_in serv_addr;
        epoll_event events[65535];
        int epollfd;
        int pending_num;

        int epoll_add(int fd);
        int epoll_del(int fd);

        struct header_t{

            int len;
        };
        ;

public :
        struct packet_t{
            packet_t(size_t readsize,const char * mess);
            packet_t() = default;
            int len;
            char content[MAXLINE];
        };
        /** TCPServer class constructor **/
        TCPServer(int port);
        ~TCPServer();

        /** Initialize a TCPServer and prepare to set up listening **/
        int init();

        /** starts the TCPServer **/
        int starts();


        /** pure event driven virtual function **/
        virtual void onShutDownConnection(int fd)=0;
        virtual void onRead(int fd,const char *, int readsize)=0;
        virtual void onAcceptConnection(int fd)= 0;
        int sendPacket(int fd,packet_t* packet);
 
};

#endif //__TCPSERVER_H_