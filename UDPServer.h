//
// Created by alan on 9/17/17.
//
#include<netinet/in.h>
#include<sys/epoll.h>
#include<vector>


#ifndef __UDPSERVER_H_
#define __UDPSERVER_H_

class UDPServer{

public :
    using message_t = struct{
        int len;
        void* content;
    };

    using udp_cb_t = int (*)(const message_t&,message_t&);

    /** UDPServer class constructor **/
    UDPServer(int port);
    ~UDPServer();

    /** Register a udp callback function **/
    int register_cb(udp_cb_t func);

    /** Initialize a TCPServer and prepare to set up listening **/
    int init();

    /** starts the TCPServer **/
    int starts();

private :
    int port;
    sockaddr_in serv_addr;
    udp_cb_t callback_func;

};

#endif //__UDPSERVER_H_
