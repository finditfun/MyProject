//
// Created by chenlei on 2020/5/2.
//

#include "Epoll.h"


Epoll::Epoll() : efd(epoll_create(1)), numActive(0) {
    exit_if(efd <= 0, "epoll_create failed");
}

Epoll::~Epoll() = default;

//将listenfd启用LT模式
//将建立的connfd启用ET模式
void Epoll::updateEvents(int _fd, int _events, int _op) const {
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = _events;
    ev.data.fd = _fd;
    printf("%s fd %d events read %d write %d\n", _op == EPOLL_CTL_MOD ? "mod" : "add", _fd, ev.events & EPOLLIN, ev.events & EPOLLOUT);
    int r = epoll_ctl(efd, _op, _fd, &ev);
    exit_if(r, "epoll_ctl failed");
}

//删除添加的描述符
void Epoll::delEvents(int _efd, int _fd) {
    int t = epoll_ctl(_efd, EPOLL_CTL_DEL, _fd, 0);
    exit_if(t == -1, "del events failed");
    close(_fd);
}

//监听描述符上面是否有事件发生
int Epoll::wait() {
    int n = epoll_wait(efd, activeEvs, kMaxEvents, -1);
    return n;
}

//获取epoll上面发生的事件，进行区分哪些fd上面有事件发生了
struct epoll_event *Epoll::getEvents() {
    return activeEvs;
}

//获取描述符
int Epoll::getFd() const {
    return efd;
};



