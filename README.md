# shiny-engine
The shiny-engine is a modular distributed game server framework. Nodes and nodes use registration to associate messages for automatic routing. You can use the tool to generate the game logic structure data code into the framework, reducing the need to manually write tedious code.

Currently supports Window 64-bit systems and linux ubuntu 64-bit systems. Communication between nodes uses protobuf, zeromq, but communication with clients uses Socket TCP. Database currently only supports mysql.

The server architecture can look at some of the files in the doc directory. The current documentation is quite simple, mainly because the author is too lazy. Of course, follow-up will continue to complete the document and continue to improve the project.

Interested friends can contact me:
Email: xqx83@hotmail.com Tencent QQ: 183708457

# Why use shiny-engine
为什么使用shiny-engine

Distributed game server development general technology is relatively difficult and development cycle is relatively long. The traditional game server development model is to customize a distributed game framework for a game, so you should think it is not a common framework to solve Repeat the development of the framework and whether the public functions can be developed in advance through the framework.
Whether it is possible to reduce the difficulty of development through an automated way, such as: automatic routing of messages, automatic update of data within the node server to the database, automatic memory management, automatic locking of multi-threaded operations, and so on.

With these problems, this framework was born.

分布式游戏服务端开发一般技术难度比较大而且开发周期比较长.传统的游戏服务端开发模式都是针对某款游戏定制一个分布式游戏框架,所以你应该会想是不是用通用的框架来解决重复开发框架的问题,并且能否通过框架的形式预先把公共的功能都开发好.
是否可以通过一种自动化的方式让开发难度降低,比如:自动路由消息,自动把节点内的数据更新到数据库,自动内存管理,多线程操作自动加锁等等.

就是带着这些问题,这套服务端框架就这样诞生了.


# How to build
如何编译

##### 1.Building Shiny-Engine for Windows
Visual C++ .NET 2010

##### 2.Building Shiny-Engine for Linux
Code::Blocks 16.01


##### 1.在Windows下编译 shiny-engine
Visual C++ .NET 2010

##### 2.在Linux 下编译 shiny-engine
Code::Blocks 16.01

# How to use
如何使用


