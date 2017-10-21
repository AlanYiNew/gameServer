#include "UDPServer.h"
#include <iostream>
#include "unistd.h"

//
// Created by alan on 9/17/17.
//
int UDPServer::init(){
    /** set protocol family ipv4 **/
    _serv_addr.sin_family = AF_INET;

    /** set address accepting any in_addr using host to network address
    transalation **/
    _serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /** set input port **/
    _serv_addr.sin_port = htons(_port);

    _callback_func = nullptr;
}

int UDPServer::register_cb(udp_cb_t func,void* arg) {
    std::cout << "registering function" << std::endl;
    _callback_func = func;
    _callback_arg = arg;
}

UDPServer::UDPServer(int p,size_t buffer_size):_port(p){
    _buffer = std::make_unique<char[]>(buffer_size);
    _buffer_size = buffer_size;
};



int UDPServer::starts() {
    sockaddr_in cliaddr;
    socklen_t cliaddrlen = sizeof(sockaddr);


    /** create a socket **/
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    int err = bind(fd, reinterpret_cast<sockaddr*>(&_serv_addr), sizeof(sockaddr_in));
    if (err < 0){
        perror("bind failed");
        exit(-1);
    }


    try {
        while (true) {
            message_t recv;

            recv.len = recvfrom(fd, _buffer.get(), _buffer_size, 0, reinterpret_cast<sockaddr*>(&cliaddr), &cliaddrlen);

            if (recv.len > 0) {
                recv.content = _buffer.get();
                if (_callback_func) {
                    message_t res;
                    _callback_func(_callback_arg,recv,res);
                    if (res.len > 0)
                        sendto(fd,res.content,res.len,0,reinterpret_cast<sockaddr*>(&cliaddr),cliaddrlen);
                }
            }
        }
    }   catch (std::exception ex){
        close(fd);
        throw ex;
    }


}

UDPServer::~UDPServer(){
}
