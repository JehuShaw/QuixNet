# QuixNet
The QuixNet is a modular distributed game server framework. Nodes and nodes use registration to associate messages for automatic routing. You can use the tool to generate the game logic structure data code into the framework, reducing the need to manually write tedious code.

Currently supports Window 64-bit systems and linux ubuntu 64-bit systems. Communication between nodes uses protobuf, zeromq, but communication with clients uses Socket TCP. Database currently only supports mysql.

The server architecture can look at some of the files in the doc directory. 

Friends who are interested or have questions can contact me :

Email: kongke84@outlook.com or xiaoqx83@gmail.com

X ：Jehu Shaw

Telegram ：xiaoqx25


## 💝 Sponsorship
If this project has been helpful to you, or you'd like to see it developed further, please consider sponsoring me via [**PayPal**](https://paypal.me/JehuShaw). Thank you so much for your support!

## 💝 赞助支持
如果这个项目对您有帮助，或者您希望看到它的进一步发展，请考虑通过 [**PayPal**](https://paypal.me/JehuShaw) 赞助我， 

当然你可以选择扫码支付：
支付宝支付：| 微信支付：
--- | ---
![支付宝收款码](sponsor_alipay.jpg) | ![微信收款码](sponsor_wxpay.jpg)

非常感谢您的支持！


# Why use QuixNet
Distributed game server development general technology is relatively difficult and development cycle is relatively long. The traditional game server development model is to customize a distributed game framework for a game, so you should think it is not a common framework to solve Repeat the development of the framework and whether the public functions can be developed in advance through the framework.
Whether it is possible to reduce the difficulty of development through an automated way, such as: automatic routing of messages, automatic update of data within the node server to the database, automatic memory management, automatic locking of multi-threaded operations, and so on.
With these problems, this framework was born.

# How to build
#### 1.Building QuixNet for Windows
##### 1).Install "Visual Studio 2022"
##### 2).Install submodule (Third party dependency libraries)Open "Developer Command Prompt for VS 2022"

\> "Your project path\submodule.bat"
##### 3). Compile code

Open "xxxx_server\vsproject\xxxx_server.sln"

#### 2.Building QuixNet for Linux
##### 1). Install "CMake 3.5+" or IDE "CLion"
##### 2). Install submodule (Third party dependency libraries). Go to your project directory and open the terminal

\> "Your project path/submodule.sh"
##### 3). Compile code，Compile using CMake and enter the following command：

\> cd xxxx_server

\> mkdir cbuild

\> cd cbuild

\> cmake -G "Unix Makefiles" -DCMAKE_SYSTEM_PROCESSOR=x86_64 -DCMAKE_BUILD_TYPE=Debug .. 

\> make -j$(nproc)
##### 4). No need to execute make install. If it is a release version, please set the parameter to  -DCMAKE_SUILD_TYPE=Release


# how to use
#### 1. Set the configuration file for the node (Like agent_server, game_server, login_server, cache_server, etc. are all called nodes.)
Each node has a configuration file "App.config" (eg： agent_server path: agent_server\bin\win32\debug\App.config).
Please set the corresponding field. The key field "ServerID" is not allowed to be duplicated.
#### 2. Install Database for CacheServer
1. Each “CacheServer” node must have a database to configure related data. (For example: mysql_script\centredb\node_memcache.sql)
2. Each “CacheServer” node must also have a database for storing data. (For example: mysql_script\centredb\node_control_centre.sql)
3. Configure the node_control_centre.sql table information to node_memcache.sql (this process is equivalent to converting the relational table to a Key-Value structure)
#### 3. GameServer is a template for all game logic nodes
The GameServer node can be expanded as a template: RankServer, BattleServer, ClanServer, and so on.
#### 4. Let the tool generate code for GameServer that can automatically update data to the database
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

#### 5. How nodes receive messages

Which receiving node's message the receiving node wants to receive requires that the receiving node's interested message be registered with the sending node.
The interior of the node is assembled through modules. The receiving node registered in the sending node is also treated as a module.
The messages processed in the module, that is, the messages that the module is interested in, also need to be listed and registered to the module management.  
Therefore, what message the node needs to register needs to be registered, and the registration message is eventually divided into two steps:  
1. Message registration between nodes  
2. Message registration between modules within a node  

Need special instructions:  
1. They handle differently in the case of no message registration, no messages are registered between nodes, all messages are received, and the modules are
No message is received without message registration.  
2. The message is ultimately processed within the module.

#### 6. Database Configuration
All nodes must connect to the database through cache_server, so the database connection configuration needs to be configured in the cache_server/src/App.config file:
    <!-- Database address -->
    <add key="DatabaseHost" value="127.0.0.1" />
    <!-- Database port -->
    <add key="DatabasePort" value="3306" />
    <!-- User name -->
    <add key="DatabaseUser" value="root" />
    <!-- Password -->
    <add key="DatabasePSW" value="xmxy08155" />
    
Install the MySQL database on Ubuntu (installing the MySQL database on Windows is not explained in detail):
\> sudo apt update
\> sudo apt install mariadb-server
\> sudo service mariadb start
\> sudo mysql_secure_installation

Create a database and import data:

cd your project directory/QuixNet/mysql_script

sudo mariadb -u root -p

Create the database

mysql\> source create_database.sql;

Import the data for the control-side cache_server (Note: The framework is divided into a control-side database and a game logic database. The control-side database manages all nodes, while the game logic database contains the game's specific business logic.)

mysql\> use node_memcache;

mysql\> source centredb/node_memcache.sql;

mysql\> use node_control_centre;

mysql\> source centredb/node_control_centre.sql;

Import the game logic cache_server (the game logic cache_server is a separate process from the control-side cache_server, requiring modification of the App.config configuration.)

mysql\> use xyproject_memcache;

mysql\> source centredb/xyproject_memcache.sql;

mysql\> use xyproject;

mysql\> source centredb/xyproject.sql;


cache_server/src/App.config Configuration Notes:
cache_server requires two databases: node_memcache.sql and node_control_centre.sql for the control node. node_memcache.sql contains configuration information required by cache_server, while node_control_centre.sql contains specific business logic data.


# 为什么使用QuixNet
分布式游戏服务端开发一般技术难度比较大而且开发周期比较长,传统的游戏服务端开发模式都是针对某款游戏定制一个分布式游戏框架,所以你应该会想是不是用通用的框架来解决重复开发框架的问题,并且能否通过框架的形式预先把公共的功能都开发好,
是否可以通过一种自动化的方式让开发难度降低,比如:自动路由消息,自动把节点内的数据更新到数据库,自动内存管理,多线程操作自动加锁等等.

就是带着这些问题,这套服务端框架就这样诞生了.

# 如何编译
#### 1.在Windows下编译 QuixNet
##### 1). 安装 Visual Studio 2022
##### 2). 安装子模块（第三方依赖库）
打开命令窗口 "Developer Command Prompt for VS 2022", 输入如下：

\> "你工程的路径\submodule.bat"

##### 3).编译代码
打开每个节点VS工程文件 "xxxx_server\vsproject\xxxx_server.sln"
#### 2.在Linux 下编译 QuixNet
##### 1). 安装 CMake 3.5 及以上版本 或者 使用图形界面IDE CLion
##### 2). 安装子模块（第三方依赖库）， 到你的工程目录打开终端“Terminal"并执行如下命令：

\> "你的工程目录/submodule.sh"

##### 3). 编译代码，使用CMake编译，输入如下命令：

\> cd xxxx_server

\> mkdir cbuild

\> cd cbuild

\> cmake -G "Unix Makefiles" -DCMAKE_SYSTEM_PROCESSOR=x86_64 -DCMAKE_BUILD_TYPE=Debug .. 

\> make -j$(nproc)
##### 4). 没必要执行 make install。如果需要编译release 版本，请修改参数为 -DCMAKE_SUILD_TYPE=Release 

# 如何使用
#### 1.为节点设置配置文件 （像 agent_server、game_server、login_server、cache_server 等等，都称为节点）
每一个节点都有一个配置文件“App.config”（比如：agent_server 路径：agent_server\bin\win32\debug\App.config),
请设置下对应的字段。其中关键字段“ServerID” 是不允许重复的。

#### 2.给CacheServer安装数据库
1. 每个CacheServer节点必须设置一个数据库用于配置相关数据。（比如：mysql_script\centredb\node_memcache.sql）
2. 每个CacheServer还必须要一个用于存储数据的数据库。（比如：mysql_script\centredb\node_control_centre.sql）
3. 把 node_control_centre.sql 表信息配置到 node_memcache.sql (这个过程相当于把关系表转换成Key-Value结构）

#### 3.GameServer 是所有游戏逻辑节点的模板
可以把 GameServer节点作为模板扩展成：排行服（RankServer）、战斗服（BattleServer)、公会服（ClanServer)等等。

#### 4.让工具给GameServer生成可以自动更新数据到数据库的代码 
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

#### 5.节点如何接收消息
接收节点要收取哪个发送节点的消息就需要把接收节点感兴趣的消息注册给发送节点。
节点内部是通过模块组合起来的，在发送节点里被注册过来的接收节点也是被当成一个模块处理的。
模块内处理的消息，也就是模块感兴趣的消息，也需要列举出来注册到模块管理。  
所以，节点接收什么消息就需要注册什么消息，注册消息最终分成两步：  
1。节点间的消息注册  
2。节点内模块间的消息注册  

需要特别说明的是：  
1。他们在没有消息注册的情况处理有所不同， 节点间在没有消息注册的情况是接收所有消息，而模块间在
没有消息注册的情况下是不接收任何消息。  
2。消息最终是在模块内被处理。  

#### 6.数据库配置
所有节点连接数据库都必须通过cache_server, 所以数据库连接配置需配置在 cache_server/src/App.config 的文件内：
    <!-- Database address -->
    <add key="DatabaseHost" value="127.0.0.1" />
    <!-- Database port -->
    <add key="DatabasePort" value="3306" />
    <!-- User name -->
    <add key="DatabaseUser" value="root" />
    <!-- Password -->
    <add key="DatabasePSW" value="xmxy08155" />

  在ubuntu下安装 MySQL 数据库 (windows 下安装mysql数据库就不具体说明了）：
\> sudo apt update
\> sudo apt install mariadb-server
\> sudo service mariadb start
\> sudo mysql_secure_installation

创建数据库并导入数据:
cd 你的工程路径/QuixNet/mysql_script
sudo mariadb -u root -p
创建数据库
mysql\> source create_database.sql;
导入控制端cache_server的数据 (说明：框架分控制端数据库和游戏逻辑数据库， 控制端是用来管理所有节点的，游戏逻辑部分就是游戏的具体业务逻辑部分。）
mysql\> use node_memcache;
mysql\> source centredb/node_memcache.sql;
mysql\> use node_control_centre;
mysql\> source centredb/node_control_centre.sql;
导入游戏逻辑cache_server (游戏逻辑的cache_server 是另外开启一个进程，和控制端cache_server 分开， 需要重新修改一下App.config配置）
mysql\> use xyproject_memcache;
mysql\> source centredb/xyproject_memcache.sql;
mysql\> use xyproject;
mysql\> source centredb/xyproject.sql;

cache_server/src/App.config 配置说明：
cache_server 需要关联两个数据库，比如：控制节点 node_memcache.sql 和 node_control_centre.sql ，node_memcache.sql 为cache_server需要知道的相关配置信息，而node_control_centre.sql为具体的业务逻辑数据

