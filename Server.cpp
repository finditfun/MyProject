//
// Created by chenlei on 2020/5/3.
//

#include "Server.h"

// 设置为非阻塞
void setNonBlock(int _fd) {
    int flags = fcntl(_fd, F_GETFL, 0);
    exit_if(flags < 0, "fcntl failed");
    int r = fcntl(_fd, F_SETFL, flags | O_NONBLOCK);
    exit_if(r < 0, "fcntl failed");
}

Server::Server(int _port) : port(_port), listen_fd(socket(AF_INET, SOCK_STREAM, 0)), httpChan(new HttpData(this)) {
    exit_if(listen_fd < 0, "socket failed");
    memset(&server_addr, 0, sizeof(server_addr));
    initBindAndListen();
}

Server::~Server() {
    close(listen_fd);
}

//初始化Server，进行bind与listen操作
void Server::initBindAndListen() {
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int r = ::bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr));
    exit_if(r, "bind to 0.0.0.0:%d failed %d %s", port, errno, strerror(errno));

    r = listen(listen_fd, 20);
    exit_if(r, "listen failed %d %s", errno, strerror(errno));

    printf("fd %d listening at %d\n", listen_fd, port);
    setNonBlock(listen_fd);

    ep.updateEvents(listen_fd, EPOLLIN | EPOLLRDHUP, EPOLL_CTL_ADD);
}

//启动Server，等待事件的到来
int Server::run() {
    for (;;) {  //实际应用应当注册信号处理函数，退出时清理资源
        int n = ep.wait();
        struct epoll_event *activeEvs = ep.getEvents();

        for (int i = 0; i < n; i++) {
            int fd = activeEvs[i].data.fd;
            int events = activeEvs[i].events;

            if (fd == listen_fd) {
                handleAccept(fd);

            } else if(events & EPOLLIN){
                    printf("new EPOLLIN event in fd: %d \n", fd);
                    httpChan->handleRead(fd);
            } else if (events & EPOLLOUT) { // 请注意，例子为了保持简洁性，没有很好的处理极端情况，例如EPOLLIN和EPOLLOUT同时到达的情况
                printf("new EPOLLOUT event in fd: %d \n", fd);
                if (true)
                    printf("handling epollout\n");
                httpChan->sendFile(fd);
            } else {
                exit_if(1, "unknown event");
            }
        }
    }
}

//处理新来的连接
void Server::handleAccept(int _fd) {
    struct sockaddr_in raddr;
    socklen_t rsz = sizeof(raddr);
    int cfd = accept(_fd, (struct sockaddr *) &raddr, &rsz);
    exit_if(cfd < 0, "accept failed");
    sockaddr_in peer, local;
    socklen_t alen = sizeof(peer);
    int r = getpeername(cfd, (sockaddr *) &peer, &alen);
    exit_if(r < 0, "getpeername failed");
    printf("accept a connection from %s\n", inet_ntoa(raddr.sin_addr));
    setNonBlock(cfd);
    ep.updateEvents(cfd, EPOLLIN |EPOLLET, EPOLL_CTL_ADD);
}

//获得该Server内的Epoll OBJ，供HttpData来使用
Epoll* Server::getEpollObj() {
    return &ep;
}