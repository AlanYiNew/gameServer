#include "UDPServer.h"
#include <iostream>
#include "unistd.h"

//
// Created by alan on 9/17/17.
//
int UDPServer::init(){
    /** set protocol family ipv4 **/
    serv_addr.sin_family = AF_INET;

    /** set address accepting any in_addr using host to network address
    transalation **/
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /** set input port **/
    serv_addr.sin_port = htons(port);

    callback_func = nullptr;
}

int UDPServer::register_cb(udp_cb_t func) {
    std::cout << "registering function" << std::endl;
    callback_func = func;
}

UDPServer::UDPServer(int port){
    this->port = port;
}

int UDPServer::starts() {
    sockaddr_in cliaddr;
    socklen_t cliaddrlen = sizeof(sockaddr);


    /** create a socket **/
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    int err = bind(fd, reinterpret_cast<sockaddr*>(&serv_addr), sizeof(sockaddr_in));
    if (err < 0){
        perror("bind failed");
        exit(-1);
    }

    char buffer[256];


    try {
        while (true) {
            message_t recv;
            recv.len = recvfrom(fd, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&cliaddr), &cliaddrlen);

            if (recv.len > 0) {
                recv.content = buffer;
                if (callback_func) {
                    message_t res;
                    callback_func(recv,res);
                    int sentbytes = sendto(fd,res.content,res.len,0,reinterpret_cast<sockaddr*>(&cliaddr),cliaddrlen);
                    std::cout << "sentbytes = " <<sentbytes << std::endl;
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
