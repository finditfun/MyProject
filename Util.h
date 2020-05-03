//
// Created by chenlei on 2020/5/3.
//

#ifndef MYPROJECT_UTIL_H
#define MYPROJECT_UTIL_H

#include <cstdio>
#include <cerrno>
#include <iostream>

#define exit_if(r, ...)                                                                          \
    if (r) {                                                                                     \
        printf(__VA_ARGS__);                                                                     \
        printf("%s:%d error no: %d error msg %s\n", __FILE__, __LINE__, errno, strerror(errno)); \
        exit(1);                                                                                 \
    }

#endif //MYPROJECT_UTIL_H
