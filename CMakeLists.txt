cmake_minimum_required(VERSION 2.14)
project(MyProject)

set(CMAKE_CXX_STANDARD 11)

add_executable(MyProject main.cpp Epoll.cpp Server.cpp Util.h HttpData.cpp HttpData.h)