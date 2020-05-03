//
// Created by chenlei on 2020/5/3.
//

#include "HttpData.h"

HttpData::HttpData() {
    httpRes = "HTTP/1.1 200 OK\r\nConnection: Keep-Alive\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: 1048576\r\n\r\n123456";
    for (int i = 0; i < 1048570; i++) {
        httpRes += '\0';
    }
}

void HttpData::handleRead(int _fd) {
    char buf[4096];
    int n = 0;
    while ((n = ::read(_fd, buf, sizeof buf)) > 0) {
        if (true)
            printf("read %d bytes\n", n);
        std::string &readed = cons[_fd].readed;
        readed.append(buf, n);
        if (readed.length() > 4) {
            if (readed.substr(readed.length() - 2, 2) == "\n\n" || readed.substr(readed.length() - 4, 4) == "\r\n\r\n") {
                //当读取到一个完整的http请求，测试发送响应
                sendRes(_fd);
            }
        }
    }//将socket中的数据读完了，但是请求可能还没有完全写入到socket（其中原因可能是请求数据太大，而socket大小有限）
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return; //返回后，socket中可能很快又会被写入数据，则又会出发epoll_wait()
    //实际应用中，n<0应当检查各类错误，如EINTR
    if (n < 0) {
        printf("read %d error: %d %s\n", _fd, errno, strerror(errno));
    }
    close(_fd);
    cons.erase(_fd);
}

void HttpData::handleWrite(int _fd) {
    sendRes(_fd);
}

void HttpData::sendRes(int _fd) {
    Con &con = cons[_fd];
    if (!con.readed.length())   //若来自client的请求没有数据，则不会给回复
        return;
    size_t left = httpRes.length() - con.written;
    int wd = 0;
    while ((wd = ::write(_fd, httpRes.data() + con.written, left)) > 0) {
        con.written += wd;
        left -= wd;
        if (true)
            printf("write %d bytes left: %lu\n", wd, left);
    };
    if (left == 0) {
        //        close(_fd); // 测试中使用了keepalive，因此不关闭连接。连接会在read事件中关闭
        cons.erase(_fd);
        return;
    }
    if (wd < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))    //缓冲区已满，需要等待socket再次变为可写
        return; //即将当前socket发送给client后，socket变为可写，然后再接着写
    if (wd <= 0) {
        printf("write error for %d: %d %s\n", _fd, errno, strerror(errno));
        close(_fd);
        cons.erase(_fd);
    }
}

