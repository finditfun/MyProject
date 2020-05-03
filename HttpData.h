//
// Created by chenlei on 2020/5/3.
//

#ifndef MYPROJECT_HTTPDATA_H
#define MYPROJECT_HTTPDATA_H

#include "Util.h"
#include <map>
#include <unistd.h>
#include <cstring>

struct Con {
    std::string readed;
    size_t written;
    Con() : written(0) {}
};
//上述为全局变量，则可以记住某个socket已经写了多少数据，接下来还需要写多少

class HttpData {
public:
    HttpData();

    ~HttpData();

    void handleRead(int _fd);

    void handleWrite(int _fd);

    void sendRes(int _fd);

private:
    std::string httpRes;
    std::map<int, Con> cons;
};


#endif //MYPROJECT_HTTPDATA_H
