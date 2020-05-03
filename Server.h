//
// Created by chenlei on 2020/5/3.
//

#ifndef MYPROJECT_SERVER_H
#define MYPROJECT_SERVER_H

#include <sys/fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include "Util.h"
#include "Epoll.h"
#include "HttpData.h"

const int max_event_num = 20;

class Server {

public:
    Server(int _port);

    ~Server();

    void initBindAndListen();

    //启动Server
    int run();

    void handleAccept(int _fd);

private:
    int port;
    int listen_fd;
    Epoll ep;
    HttpData* httpChan;
    struct sockaddr_in server_addr;
};


#endif //MYPROJECT_SERVER_H
