//
// Created by chenlei on 2020/5/3.
//

#ifndef MYPROJECT_HTTPDATA_H
#define MYPROJECT_HTTPDATA_H

#include "Util.h"
#include <map>
#include <unistd.h>
#include <cstring>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>

struct Con {    //记忆每个连接的基本信息，用一个map<int, Con>映射，根据fd可以找到对应的Con
    std::string filename;   //文件名
    std::string type;
    int num;
    std::string info;
    std::string readed; //用于记录handleRead读了多少数据，因为要分批次读取
    size_t written; //已经写了多少数据

    Con() : num(200), info("OK"), written(0){}
};
//上述为全局变量，则可以记住某个socket已经写了多少数据，接下来还需要写多少
class Server;
class HttpData {
public:
    HttpData();

    explicit HttpData(class Server* _server);

    ~HttpData();

    void handleRead(int _fd);

    void handleWrite(int _fd);

    void sendRes(int _fd);

    void sendFile(int _fd);

    void dealGet(int _fd, const std::string & uri, int start);  // 处理GET请求

    void dealPost(int _fd, const std::string & uri, char *buf );  // 处理POST请求

private:
    std::string httpRes;
    std::map<int, Con> cons;
    class Server* server;
};


#endif //MYPROJECT_HTTPDATA_H
