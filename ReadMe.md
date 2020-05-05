## MyServer
### 0 本仓库由来

作为一个菜鸟，我一直想写出自己的Web Server。

在github上学习了好多作者的代码，受益匪浅。从star为0的个人练手项目到star为几千的项目，不同作者有不同的思维。

我花费最长时间看的应该是muduo网络库了，的确有好多启发，但也有好多地方一知半解。

在我琢磨别人的代码时，对epoll中的ET与LT模式下，server端如何进行调节的问题，我发现知乎上有个高赞回答，答者贴出了https://github.com/yedf/handy/blob/master/raw-examples/epoll-et.cc路径，我带着好奇打开，并没有对其抱有太大希望。

然而作者短短的代码却如同醍醐灌顶，让我豁然开朗。

我想，何不以作者的代码为模板，逐渐改造一个完整的WebServer呢？在改进中逐渐加深对知识的了解。

所以我新建该仓库，以后逐渐完善。希望有一天，这个项目能够变得强大起来吧。

### 1 第一版叙述

我的第一版Web Server是在学习https://github.com/yedf/handy/blob/master/raw-examples/epoll-et.cc 的基础上修改的，主要做的就是将作者一个文件中的代码逻辑用不同类来进行封装，以便后续的扩展，没有什么自己的东西。

#### 1.1 代码结构如下：

```
MyServer
├── CMakeLists.txt	//cmake编译文件
├── Epoll.cpp	//封装epoll
├── Epoll.h
├── HttpData.cpp	//对client的请求进行分析与回应
├── HttpData.h
├── main.cpp	//主程序
├── Server.cpp	//封装一个Server需要具备的成员属性与成员函数
├── Server.h
└── Util.h	//简单封装一个log打印宏
```

这版的Server对请求只是简单地读取到client发送来的数据，不进行分析，最后将Server构造函数生成的1M数据发送给client。

#### 1.2 程序逻辑：

1. main.cpp创建Server对象server；
2. server构造函数生成一个listenfd，并进行bind、listen操作，用来监听client来的请求；
3. server调用run函数，开启一个循环，由于Epoll类是Server类的一部分，所以，server循环中wait在epoll_wait()，当有事件到达，epoll_wait()返回；
4. server中包含指向HttpData对象的指针，对epoll_wait()返回的结果进行分析，若是listenfd有事件发生，则交于server的handleAccept()进行处理，若是已建立连接的socketfd有事件发生，则需要区分是读事件还是写事件，分别交于HttpData类中的handleRead()与handleWrite()进行处理。

之所以epoll_wait()返回的已建立连接的socketfd有写事件发生，是因为：

这儿的epoll采用了ET模式，ET模式的特点就是当fd上面有事件发生，会进行通知，若服务器没有及时处理完毕，则后续epoll也不会进行通知（ET模式下，仅仅是socketfd状态发生改变——缓冲区空/非空的转变时才会进行通知）。

比如，你要给socketfd写数据，但是写了一些，socketfd的缓冲区已经满了，这样socketfd中的数据就通过底层进行发送数据，你下次给这个socketfd中写数据的时候需要当该socketfd缓冲区的数据已经全部发送出去（非空变为空），这样又会触发epoll_wait()，这样我们才能接着给该socketfd中写剩下的数据。（handy作者的思路）

#### 1.3 不足

上述思路对应着我们在处理事件的过程中，我们只能单线程/进程进行处理。因为涉及到要重复对epoll_wait()返回的socketfd进行写操作，所以我们需要建立对某个socketfd的映射，某个socketfd再次返回时，我们需要找到上次的线程进行继续读写。（这儿或许可以建立一种socketfd到线程id的映射机制，这样在多线程中也可以使用）

某有对client的请求进行分析与处理，返回的数据也比较单一。

这些即下一版本的重点攻克方向。



### 2 第二版叙述

第二版主要优化对client请求处理，对其请求进行分析并返回多媒体文件给客户端，此外引入MySQL处理程序来进行账户的验证等操作。

