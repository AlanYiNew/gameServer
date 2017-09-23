#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <unistd.h>
#include "TCPServer.h"
#include <algorithm>

#define MAXLINE 4096

#define TEST_H
using namespace ::std;

static int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);

TCPServer::TCPServer(int port) {
    this->port = port;
}

int TCPServer::init() {

    /** set protocol family ipv4 **/
    serv_addr.sin_family = AF_INET;

    /** set address accepting any in_addr using host to network address 
    transalation **/
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /** set input port **/
    serv_addr.sin_port = htons(this->port);


    return 0;
}

int TCPServer::starts() {
    /** create a socket **/
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    /** buff **/
    char buff[MAXLINE + 1];

    int err = bind(listenfd, (sockaddr*)&serv_addr, sizeof(sockaddr_in));
    err = listen(listenfd, pending_num);
    socklen_t len = sizeof(sockaddr_in);

    /** creates epollfd **/
    if ((epollfd = epoll_create(EPOLL_CLOEXEC)) < 0)
        throw runtime_error("error in creating epoll fd");

    /** add listenfd **/
    err = epoll_add(listenfd);

    while (true) {
        /** wait on the events list and never timeout
          * kind of like blokcing on all events **/
        int nready = epoll_wait(epollfd, events, pending_num, -1);

        for (int i = 0; i < nready; i++) {
            if (events[i].data.fd == listenfd) {
                int connfd = accept(listenfd, (struct sockaddr *) &serv_addr, &len);
                epoll_add(connfd);
                onAcceptConnection(connfd);
            } else if (events[i].events & EPOLLIN) {
                int readsize = 0;
                if ((readsize = recv(events[i].data.fd, buff, sizeof(_header), MSG_WAITALL)) < 0) {
                    throw std::runtime_error("error during reading packet header from socket");
                } else if (readsize == 0) {
                    shutdown(events[i].data.fd, SHUT_RDWR);
                    close(events[i].data.fd);
                    epoll_del(events[i].data.fd);
                    onShutDownConnection(events[i].data.fd);

                } else {
                    _header * h = reinterpret_cast<_header*>(buff);
                    if (readsize = recv(events[i].data.fd, buff+sizeof(_header), h->len, MSG_WAITALL)){
                        onRead(events[i].data.fd,buff+sizeof(_header),readsize);
                    }   else{
                        throw std::runtime_error("error during reading packet content from socket");
                    }

                }

            }
        }
    }


    return 0;
}

TCPServer::~TCPServer() {

}

int TCPServer::epoll_del(int fd) {
    return epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr);
}

int TCPServer::epoll_add(int fd) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    return epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}

int TCPServer::send(int fd,char * message,int size){
    send(fd,message,size);
}