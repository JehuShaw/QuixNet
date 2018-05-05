# shiny-engine
The shiny-engine is a modular distributed game server framework. Nodes and nodes use registration to associate messages for automatic routing. You can use the tool to generate the game logic structure data code into the framework, reducing the need to manually write tedious code.

Currently supports Window 64-bit systems and linux ubuntu 64-bit systems. Communication between nodes uses protobuf, zeromq, but communication with clients uses Socket TCP. Database currently only supports mysql.

The server architecture can look at some of the files in the doc directory. The current documentation is quite simple, mainly because the author is too lazy. Of course, follow-up will continue to complete the document and continue to improve the project.

Interested friends can contact me:
Email: xqx83@hotmail.com Tencent QQ: 183708457

# Why use shiny-engine
Distributed game server development general technology is relatively difficult and development cycle is relatively long. The traditional game server development model is to customize a distributed game framework for a game, so you should think it is not a common framework to solve Repeat the development of the framework and whether the public functions can be developed in advance through the framework.
Whether it is possible to reduce the difficulty of development through an automated way, such as: automatic routing of messages, automatic update of data within the node server to the database, automatic memory management, automatic locking of multi-threaded operations, and so on.

With these problems, this framework was born.

# 为什么使用shiny-engine
分布式游戏服务端开发一般技术难度比较大而且开发周期比较长.传统的游戏服务端开发模式都是针对某款游戏定制一个分布式游戏框架,所以你应该会想是不是用通用的框架来解决重复开发框架的问题,并且能否通过框架的形式预先把公共的功能都开发好.
是否可以通过一种自动化的方式让开发难度降低,比如:自动路由消息,自动把节点内的数据更新到数据库,自动内存管理,多线程操作自动加锁等等.

就是带着这些问题,这套服务端框架就这样诞生了.


# How to build
#### 1.Building Shiny-Engine for Windows
Visual C++ .NET 2010

#### 2.Building Shiny-Engine for Linux
Code::Blocks 16.01

# 如何编译
#### 1.在Windows下编译 shiny-engine
Visual C++ .NET 2010

#### 2.在Linux 下编译 shiny-engine
Code::Blocks 16.01

# how to use
#### 1. Set the configuration file
Each node has a configuration file "App.config" (eg： agent_server path: agent_server\bin\win32\debug\App.config).
Please set the corresponding field. The key field "ServerID" is not allowed to be duplicated.
#### 2. Install Database for CacheServer
1. Each “CacheServer” node must have a database to configure related data. (For example: mysql_script\centredb\node_memcache.sql)
2. Each “CacheServer” node must also have a database for storing data. (For example: mysql_script\centredb\node_control_centre.sql)
3. Configure the node_control_centre.sql table information to node_memcache.sql (this process is equivalent to converting the relational table to a Key-Value structure)
#### 3. Let the tool generate code for GameServer that can automatically update data to the database
Use the tools\mc.rar tool to list fields in DataCodeGenerate\in.txt, such as:

// Character account ID  
uint64_t account;  
// Character Name  
string name;  
// Character level  
int32_t level;  
// Character experience  
int32_t exp;  

Then double-click DataCodeGenerate\Build.bat to get the code.

# 如何使用
#### 1.设置配置文件
每一个节点都有一个配置文件“App.config”（比如：agent_server 路径：agent_server\bin\win32\debug\App.config),
请设置下对应的字段。其中关键字段“ServerID” 是不允许重复的。

#### 2.给CacheServer安装数据库
1. 每个CacheServer节点必须设置一个数据库用于配置相关数据。（比如：mysql_script\centredb\node_memcache.sql）
2. 每个CacheServer还必须要一个用于存储数据的数据库。（比如：mysql_script\centredb\node_control_centre.sql）
3. 把 node_control_centre.sql 表信息配置到 node_memcache.sql (这个过程相当于把关系表转换成Key-Value结构）

#### 3.让工具给GameServer生成可以自动更新数据到数据库的代码 
使用 tools\mc.rar 工具，在 DataCodeGenerate\in.txt 列举出字段，比如:

// 角色账号ID  
uint64_t account;  
// 角色名称  
string name;  
// 角色等级  
int32_t level;  
// 角色经验  
int32_t exp;  

然后鼠标双击 DataCodeGenerate\Build.bat 获得代码。
