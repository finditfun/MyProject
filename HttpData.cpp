//
// Created by chenlei on 2020/5/3.
//

#include <sys/fcntl.h>
#include "HttpData.h"
#include "Server.h"

HttpData::HttpData() {

}

HttpData::HttpData(class Server* _server) {
    server = _server;
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
            if (readed.substr(readed.length() - 2, 2) == "\n\n" ||
                readed.substr(readed.length() - 4, 4) == "\r\n\r\n") {
                //当读取到一个完整的http请求，测试发送响应
                int start = 0;
                char method[5], uri[100], version[10];
                sscanf( readed.c_str(), "%s %s %s", method, uri, version );

//                if( char *tmp = strstr( readed.c_str(), "Range:" ) ) {
//                    tmp += 13;  //Range：Range头域可以请求实体的一个或者多个子范围。服务器可以忽略此请求头
//                    sscanf( tmp, "%d", &start );
//                }
                server->getEpollObj()->updateEvents(_fd, EPOLLOUT, EPOLL_CTL_MOD);
                if( !strcmp( method, "GET" ) ) {  // 为GET
                    dealGet(_fd, uri, start );
                } else if( !strcmp( method, "POST" ) ) {  // 为POST
                    dealPost(_fd, uri, buf );
                } else {
                    const char *header = "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/plain;charset=utf-8\r\n\r\n";
                    send( _fd, header, strlen(header), 0 );
                }
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
    struct stat filestat;
    int ret = stat(cons[_fd].filename.c_str(), &filestat);
    if (ret < 0 && cons[_fd].filename.c_str() || !S_ISREG(filestat.st_mode)) {  // 打开文件出错或没有该文件
        printf("file not found : %s\n", cons[_fd].filename.c_str());
        cons[_fd].filename = "html/404.html";
        cons[_fd].type = "text/html";
        cons[_fd].num = 404;
        cons[_fd].info = "Not Found";
        sendFile(_fd);
        return;
    }

    char header[200];
    sprintf(header,
            "HTTP/1.1 %d %s\r\nServer: finditfun\r\nContent-Length: %d\r\nContent-Type: %s;charset:utf-8\r\n\r\n", cons[_fd].num,
            cons[_fd].info.c_str(), int(filestat.st_size), cons[_fd].type.c_str());

    // send第二个参数只能是c类型字符串，不能使用string
    send(_fd, header, strlen(header), 0);
    sendFile(_fd);
}

void HttpData::dealGet(int _fd, const std::string &uri, int start = 0) {

    cons[_fd].filename = uri.substr(1); //此处每个fd已经映射了其对应的filename

    if (uri == "/" || uri == "/index.html") {
        cons[_fd].filename = "index.html";
        cons[_fd].type = "text/html";
        handleWrite(_fd);
    } else if (uri.find(".jpg") != std::string::npos || uri.find(".png") != std::string::npos) {
        cons[_fd].type = "image/jpg";
        handleWrite(_fd);
    } else if (uri.find(".html") != std::string::npos) {
        cons[_fd].type = "text/html";
        handleWrite(_fd);
    } else if (uri.find(".ico") != std::string::npos) {
        cons[_fd].type = "image/x-icon";
        handleWrite(_fd);
    } else if (uri.find(".js") != std::string::npos) {
        cons[_fd].type = "yexy/javascript";
        handleWrite(_fd);
    } else if (uri.find(".css") != std::string::npos) {
        cons[_fd].type = "text/css";
        handleWrite(_fd);
    } else if (uri.find(".mp3") != std::string::npos) {
        cons[_fd].type = "audio/mp3";
        handleWrite(_fd);
    } else if (uri.find(".mp4") != std::string::npos) {
        cons[_fd].type = "audio/mp4";
        handleWrite(_fd);
    } else {
        cons[_fd].type = "text/plain";
        handleWrite(_fd);
    }
}

void HttpData::dealPost(int _fd, const std::string &uri, char *buf) {
    std::string filename = uri.substr(1);
    if (uri.find("adder") != std::string::npos) {  //使用CGI服务器，进行加法运算
        char *tmp = buf;
        int len, a, b;
        char *l = strstr(tmp, "Content-Length:");  // 获取请求报文主体大小
        sscanf(l, "Content-Length: %d", &len);
        len = strlen(tmp) - len;
        tmp += len;
        sscanf(tmp, "a=%d&b=%d", &a, &b);
        sprintf(tmp, "%d+%d,%d", a, b, _fd);  // tmp存储发送到CGI服务器的参数

        // fork产生子进程，执行CGI服务器进行计算（webServer一眼只进行解析、发送数据，不进行相关计算）
        if (fork() == 0) {
            // dup2( accp_fd, STDOUT_FILENO );
            execl(filename.c_str(), tmp, NULL);
        }
        wait(NULL);  // 等待子进程结束
    } else {
        cons[_fd].filename = "html/404.html";
        cons[_fd].type = "text/html";
        cons[_fd].num = 404;
        cons[_fd].info = "Not Found";
        sendFile(_fd);
    }
}

void HttpData::sendFile(int _fd) {
    struct stat filestat;
    stat(cons[_fd].filename.c_str(), &filestat);
    int fd = open(cons[_fd].filename.c_str(), O_RDONLY);

    Con &con = cons[_fd];
    if (!con.readed.length())   //若来自client的请求没有数据，则不会给回复
        return;
    size_t left = filestat.st_size - con.written;   //用于记忆给_fd中写了多少数据，还剩多少数据

    off_t t = con.written;

    ssize_t wd = 0;
    while ((wd = sendfile(_fd, fd, &t, filestat.st_size)) > 0) {
        con.written += wd;
        left -= wd;
        if (true)
                printf("write %d bytes left: %lu\n", wd, left);
    }
    if (left == 0) {
        //        close(_fd); // 测试中使用了keepalive，因此不关闭连接。连接会在read事件中关闭
        server->getEpollObj()->updateEvents(_fd, EPOLLIN, EPOLL_CTL_MOD);
        cons.erase(_fd);
        return;
    }
    if (wd < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {    //缓冲区已满，需要等待socket再次变为可写
        server->getEpollObj()->updateEvents(_fd, EPOLLOUT, EPOLL_CTL_MOD);
        return; //即将当前socket发送给client后，socket变为可写，然后再接着写
     }
    if (wd <= 0) {
        printf("write error for %d: %d %s\n", _fd, errno, strerror(errno));
        close(_fd);
        cons.erase(_fd);
    }
    close(fd);
}

