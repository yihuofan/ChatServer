ChatServer - 基于 C++ 和 Muduo 的集群聊天服务器

使用 C++11 编写，基于 Muduo 网络库的高性能集群聊天服务器项目。

实现了一个完整的即时通讯后端服务，支持用户管理、一对一聊天、群组聊天、好友管理和离线消息处理等核心功能。

引入 Redis 的发布/订阅功能，该项目具备支持多服务器部署的能力，可以轻松扩展为集群模式，以应对高并发场景。

## 功能特性

- **用户管理**: 支持用户注册、登录和注销。
- **状态同步**: 客户端可获取好友的在线状态。
- **即时通讯**:
    - **一对一聊天**: 支持用户间的私聊。
    - **群组聊天**: 支持多人群聊。
- **好友管理**:
    - 添加好友。
    - 查看好友列表。
- **群组功能**:
    - 创建群组。
    - 加入群组。
    - 查看群组列表和群成员。
- **离线消息**: 用户上线后可接收离线期间的个人和群组消息。
- **集群支持**: 通过 Redis 的发布/订阅模式，支持在多个服务器实例之间转发消息，实现服务端的横向扩展。

## 技术栈

- **语言**: C++11
- **网络库**: [Muduo](https://github.com/chenshuo/muduo) - 一个基于 Reactor 模式的现代化 C++ 网络库，用于处理高并发网络连接。
- **数据库**: MySQL - 用于持久化存储用户信息、好友关系、群组信息等。
- **消息队列/缓存**: Redis - 使用其发布/订阅（Pub/Sub）功能实现跨服务器的事件通知和消息转发。
- **数据序列化**: [JSON for Modern C++](https://github.com/nlohmann/json) - 用于客户端和服务器之间的数据交换格式。
- **构建系统**: CMake - 用于跨平台的项目构建管理。

## 架构设计

项目采用分层设计，将网络层、业务逻辑层和数据层解耦：
1.  **网络层**: 基于 Muduo 库，负责处理 TCP 连接、网络 I/O 事件和线程分发。`ChatServer` 类封装了服务器的启动和网络事件回调。
2.  **业务逻辑层**: `ChatService` 作为核心业务处理类（单例模式），负责分发不同消息类型的业务逻辑，如登录、注册、聊天等。
3.  **数据层**:
    - **MySQL**: `MySQL` 类提供数据库连接和操作接口。`UserModel`、`FriendModel`、`GroupModel` 和 `OfflineMsgModel` 等类封装了对特定数据表的增删改查（DAO 模式），实现了业务与数据库的隔离。
    - **Redis**: `Redis` 类封装了 hiredis 客户端，用于实现跨服务器的发布/订阅功能，处理集群间的消息同步。



## 开始使用

### 1. 环境依赖

在编译和运行本项目之前，请确保已安装以下依赖项：

- **CMake** (>= 3.0)
- **GCC** (>= 4.8, 支持 C++11)
- **Muduo** 网络库
- **MySQL** 开发库 (`libmysqlclient-dev`)
- **Redis** 开发库 (`libhiredis-dev`)
- 运行中的 **MySQL Server**
- 运行中的 **Redis Server**

在 Ubuntu/Debian 系统上，您可以使用以下命令安装开发库：
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libmysqlclient-dev libhiredis-dev
```
*您需要自行编译安装 Muduo 库。*

### 2. 数据库设置

本项目需要一个名为 `chat` 的数据库。请在您的 MySQL 服务器中创建该数据库，并执行以下 SQL 脚本以创建所需的表结构：

```sql
CREATE DATABASE chat;
USE chat;

-- 用户表
CREATE TABLE `user` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(50) NOT NULL UNIQUE,
  `password` VARCHAR(50) NOT NULL,
  `state` ENUM('online', 'offline') DEFAULT 'offline',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 好友关系表
CREATE TABLE `friend` (
  `userid` INT NOT NULL,
  `friendid` INT NOT NULL,
  PRIMARY KEY (`userid`, `friendid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 群组信息表
CREATE TABLE `allgroup` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `groupname` VARCHAR(50) NOT NULL UNIQUE,
  `groupdesc` VARCHAR(200) DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 群组成员关系表
CREATE TABLE `groupuser` (
  `groupid` INT NOT NULL,
  `userid` INT NOT NULL,
  `grouprole` ENUM('creator', 'normal') DEFAULT 'normal',
  PRIMARY KEY (`groupid`, `userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 离线消息表
CREATE TABLE `offlinemessage` (
  `userid` INT NOT NULL,
  `message` TEXT NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

### 3. 编译

项目使用 CMake 进行构建。请按照以下步骤进行编译：

```bash
# 1. 克隆仓库 (如果需要)
# git clone <your-repo-url>
# cd ChatServer

# 2. 创建并进入 build 目录
mkdir build
cd build

# 3. 运行 CMake 和 Make
cmake ..
make
```
编译成功后，可执行文件 `ChatServer` 和 `ChatClient` 将生成在 `bin` 目录下。

### 4. 运行

1.  **启动服务器**:
    ```bash
    ./bin/ChatServer <ip> <port>
    # 示例:
    ./bin/ChatServer 127.0.0.1 6000
    ```

2.  **启动客户端**:
    打开一个新的终端，启动客户端并连接到服务器。
    ```bash
    ./bin/ChatClient <server_ip> <server_port>
    # 示例:
    ./bin/ChatClient 127.0.0.1 6000
    ```

## 客户端使用指南

客户端是一个命令行交互程序。

1.  **主菜单**:
    - `1`: 登录
    - `2`: 注册
    - `3`: 退出

2.  **登录后**:
    登录成功后，您会看到好友列表、群组列表和离线消息。之后，您可以输入以下命令进行操作：

    - `help`: 显示所有支持的命令及其格式。
    - `chat:friendid:message`: 与指定 ID 的好友进行一对一聊天。
      - > 示例: `chat:2:你好啊`
    - `addfriend:friendid`: 添加指定 ID 的用户为好友。
      - > 示例: `addfriend:3`
    - `creategroup:groupname:groupdesc`: 创建一个新群组。
      - > 示例: `creategroup:技术交流群:欢迎大家加入`
    - `addgroup:groupid`: 加入指定 ID 的群组。
      - > 示例: `addgroup:1`
    - `groupchat:groupid:message`: 在指定群组中发送消息。
      - > 示例: `groupchat:1:今天天气不错`
    - `loginout`: 注销当前用户，返回主菜单。
