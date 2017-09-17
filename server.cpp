#include <iostream>
#include <string.h>
#include <vector>
#include <exception>
#include <unistd.h>
#include <cstdlib>
#include "server.h"
#include <pthread.h>
#include <cstdio>
#include <errno.h>
#include <sys/time.h>

#define MAXLINE 4096

#define TEST_H
using namespace::std;

static int timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y);

Server::Server(int port,int event_num){
    this->port = port;
    this->event_num = event_num;
}

int Server::init(){
    /** declare a socketaddr_in **/       
    this->serv_addr = new sockaddr_in;
       
    /** set protocol family ipv4 **/
    this->serv_addr->sin_family=AF_INET;
    
    /** set address accepting any in_addr using host to network address 
    transalation **/
    this->serv_addr->sin_addr.s_addr=htonl(INADDR_ANY);
    
    /** set input port **/
    serv_addr->sin_port=htons(this->port);

    /** event listening array **/
    this->events = new epoll_event[this->event_num];

    /** init vector to keep track of client fds **/
    this->clientfds.resize(this->event_num);
   
    return 0;
}

int Server::starts(){
    /** create a socket **/
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    
    /** buff **/
    char buff[MAXLINE+1];
   
    
 
    try {
    
        int err = bind(listenfd,(sockaddr*)(serv_addr),sizeof(sockaddr_in));
        
        err =  listen(listenfd,event_num);        
       
        socklen_t len = sizeof(sockaddr_in); 

        /** creates epollfd **/
	int epollfd;  
 
	if ((epollfd = epoll_create(EPOLL_CLOEXEC)) < 0){
	    perror("epollfd creates fails");
	    ::exit(-1);
        }

        /** create event for listrener **/
        epoll_event event;
        event.data.fd = listenfd;
        event.events = EPOLLIN; 

	/** Add it into event list **/
        if (epoll_ctl(epollfd, EPOLL_CTL_ADD,listenfd,&event) < 0){
	    perror("listenfd added fails");
	    ::exit(-1);
        }

#ifdef TEST_H
        int time = 0;
        struct timeval start, stop, diff;
#endif
        while (true){
            /** wait on the events list and never timeout
              * kind of like blokcing on all events **/
            int nready = epoll_wait(epollfd,events,event_num,-1); 
#ifdef TEST_H              
            /** test code **/
            if (!time) {
                time = 1;
                gettimeofday(&start, 0);            
            }
#endif

            for (int i = 0; i < nready; i++){
                if (events[i].data.fd == listenfd){
		    int connfd = accept(listenfd,(struct sockaddr*)serv_addr,&len);
                    
                    event.data.fd = connfd;
                    event.events = EPOLLIN;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD,connfd,&event) < 0){
	                perror("clientfd added fails");
	                ::exit(-1);
                    }
                    clientfds.push_back(connfd);
                }   else if (events[i].events & EPOLLIN){
                    int readsize = 0;
                    if ((readsize = recv(events[i].data.fd,buff,MAXLINE,MSG_WAITALL)) < 0){
                       if (errno==ECONNRESET)  
                          close(events[i].data.fd);  
                       else  
                          perror("read error");  
                          ::exit(-1);
                    }	else if (readsize == 0){
		        shutdown(events[i].data.fd,SHUT_RDWR);	
                        close(events[i].data.fd);
                        epoll_ctl(epollfd, EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
                        for (vector<int>::iterator it = clientfds.begin();it != clientfds.end();){
                            if (*it == events[i].data.fd){
                               clientfds.erase(it);
                               break;
                            }   else{
                               it++;
                            }
                        }

#ifdef TEST_H 
                        /** test code **/ 
                        /** teminates signal **/
                        if (buff[0] == 't'){
                           gettimeofday(&stop, 0);
                           ::timeval_subtract(&diff,&start,&stop);  
                           cout << diff.tv_usec << endl;
                        }
#endif

                    }   else {
                        buff[readsize] = '\0'; 
                    }
                    
                }
            }

        }
    }   catch (exception& e){

        cout << e.what() << endl; 

        return -1;
    }
    
    return 0;
}

Server::~Server(){
    if (serv_addr != NULL){
       delete(serv_addr);
       delete(events); 
    }
}

#ifdef TEST_H
static int timeval_subtract(struct timeval* result, struct timeval* x, struct timeval* y){   
        int   nsec;   
    
        if   (   x->tv_sec>y->tv_sec   )   
                  return   -1;   
    
        if   (   (x->tv_sec==y->tv_sec)   &&   (x->tv_usec>y->tv_usec)   )   
                  return   -1;   
    
        result->tv_sec   =   (   y->tv_sec-x->tv_sec   );   
        result->tv_usec   =   (   y->tv_usec-x->tv_usec   );   
    
        if   (result->tv_usec<0)   
        {   
                  result->tv_sec--;   
                  result->tv_usec+=1000000;   
        }   
    
        return   0;   
} 
#endif  
