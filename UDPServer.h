//
// Created by alan on 9/17/17.
//
#include<netinet/in.h>
#include<sys/epoll.h>
#include<vector>
#include<memory>


#ifndef __UDPSERVER_H_
#define __UDPSERVER_H_

class UDPServer{

public :
    using message_t = struct{
        int len;
        void* content;
    };

    using udp_cb_t = int (*)(void* userptr,const message_t&,message_t&);

    /** UDPServer class constructor **/
    UDPServer(int port,size_t buffer_size);
    ~UDPServer();

    /** Register a udp callback function **/
    int register_cb(udp_cb_t func, void * arg);

    /** Initialize a TCPServer and prepare to set up listening **/
    int init();

    /** starts the TCPServer **/
    int starts();

private :
    uint16_t _port;
    sockaddr_in _serv_addr;
    udp_cb_t _callback_func;
    std::unique_ptr<char[]> _buffer;
    size_t _buffer_size;
    void* _callback_arg;
};

#endif //__UDPSERVER_H_
