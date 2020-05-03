/*
 * 编译：c++ -o epoll-et epoll-et.cc
 * 运行： ./epoll-et
 * 测试：curl -v localhost
 * 客户端发送GET请求后，服务器返回1M的数据，会触发EPOLLOUT，从epoll-et输出的日志看，EPOLLOUT事件得到了正确的处理
 */
#include <arpa/inet.h>
#include <csignal>
#include <sys/epoll.h>
#include <string>
#include "Server.h"

int main(int argc, const char *argv[]) {
    if (argc > 1) {
        printf("no extra arg, just ./main");
    }
    ::signal(SIGPIPE, SIG_IGN);
    unsigned short port = 8888;

    Server server(port);
    server.run();

    return 0;
}
