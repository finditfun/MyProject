//
// Created by chenlei on 2020/5/2.
//

#ifndef MYPROJECT_EPOLL_H
#define MYPROJECT_EPOLL_H


#include <sys/epoll.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <cstring>
#include "Util.h"


class Epoll {
public:
    Epoll();

    ~Epoll();

    //涉及add fd与update fd
    void updateEvents(int _fd, int _events, int _op) const;

    //del fd
    void delEvents(int _efd, int _fd);

    int wait();

    struct epoll_event* getEvents();
    int getFd() const;

private:
    int efd;
    int numActive;
    static const int kMaxEvents = 100000;
    struct epoll_event activeEvs[kMaxEvents];
};


#endif //MYPROJECT_EPOLL_H
