# C++ 高性能集群聊天服务器 (High-Performance C++ Cluster Chat Server)

这是一个基于 C++11 和 Muduo 网络库实现的高性能、可集群部署的聊天服务器项目。它利用 Nginx 实现负载均衡，并借助 Redis 的发布/订阅功能实现服务器间的通信，从而支持分布式部署，承载海量用户并发。

---

### 核心特性

*   **用户服务**: 支持用户注册、登录和注销。
*   **好友管理**: 支持添加好友和展示好友列表。
*   **一对一聊天**: 实现实时私聊，并具备离线消息存储与上线后推送功能。
*   **群组功能**: 支持创建群组、加入群组、查看群组成员和群聊。
*   **集群部署**: 允许多个服务器实例并行运行，通过 Nginx 进行负载均衡。
*   **跨服务器通信**: 使用 Redis 的发布/订阅模型，实现不同服务器节点上用户之间的无缝通信。

---

### 系统架构

本项目采用分布式架构，旨在通过横向扩展服务器实例来承载大量并发用户连接。

**架构图:**
```
      +------------------+
      |      Clients     |
      +--------+---------+
               |
               | TCP Connection
               v
      +------------------+
      |      Nginx       | (TCP Load Balancer)
      +--+-------------+-+
         |             |
      ...|...       ...|... (Round-robin)
         |             |
+--------v---------+ +--------v---------+
|  ChatServer (1)  | |  ChatServer (2)  | ...
+--------+---------+ +--------+---------+
         |                      |
         |----------------------+------> |                      | (MySQL & Redis access)
         |                      |
+--------v---------+ +--------v---------+
|      MySQL       | |      Redis       |
|  (User Data,     | |  (Pub/Sub for    |
|   Groups, etc.)  | |  Cross-Server    |
|                  | |  Communication)  |
+------------------+ +------------------+
```

**组件说明:**

1.  **客户端 (Client)**:
    *   使用 C++ 编写的终端客户端，负责与用户交互。
    *   通过 Nginx 代理的 IP 和端口连接到服务器集群，对集群部署无感知。

2.  **Nginx (负载均衡器)**:
    *   作为 TCP 流量的负载均衡器，工作在 `stream` 模式下。
    *   将客户端的连接请求通过轮询 (round-robin) 或其他策略分发到后端的多个 ChatServer 实例。
    *   对客户端透明，客户端只需知道 Nginx 的地址。

3.  **聊天服务器 (ChatServer)**:
    *   项目核心，基于 Muduo 网络库实现，处理用户的具体业务逻辑（登录、聊天、群组等）。
    *   每个服务器实例维护一部分在线用户的 TCP 连接。
    *   服务器实例之间是解耦的，它们不直接通信，而是通过 Redis 消息中间件进行协作。

4.  **MySQL (持久化存储)**:
    *   作为主数据库，负责存储永久性数据，如用户信息、密码、好友关系、群组信息、离线消息等。

5.  **Redis (消息中间件)**:
    *   核心功能是利用其 **发布/订阅 (Pub/Sub)** 机制实现跨服务器通信。
    *   **场景**: 用户 A（连接在 Server 1）向用户 B（连接在 Server 2）发送消息。
        1.  Server 1 收到消息，在本地连接表中查找用户 B，未找到。
        2.  Server 1 将消息 **发布 (PUBLISH)** 到以用户 B 的 ID 命名的 Redis 频道上。
        3.  Server 2 在启动时，为其上连接的每个用户都 **订阅 (SUBSCRIBE)** 了相应的 Redis 频道。因此，Server 2 会收到该消息。
        4.  Server 2 从 Redis 接收到消息后，查找本地连接表，找到用户 B 的连接，并将消息转发给用户 B 的客户端。
    *   这种设计使得 ChatServer 实例可以成为无状态的应用节点，易于水平扩展。

---

### 技术栈

*   **语言**: C++11
*   **网络库**: [Muduo](https://github.com/chenshuo/muduo) (一个基于 Reactor 模式的现代化 C++ 网络库)
*   **数据库**: MySQL
*   **消息中间件**: Redis (主要使用其 Pub/Sub 功能)
*   **负载均衡**: Nginx (Stream 模块)
*   **构建系统**: CMake
*   **JSON 库**: [nlohmann/json](https://github.com/nlohmann/json)

---

### 环境准备与依赖

在编译前，请确保已安装以下依赖库：

*   GCC (g++ >= 4.8) 或 Clang
*   CMake (>= 3.0)
*   Muduo 库
*   MySQL 开发库 (`libmysqlclient-dev` on Ubuntu/Debian)
*   Hiredis 开发库 (`libhiredis-dev` on Ubuntu/Debian)
*   Nginx (用于集群部署)
*   Redis 服务器

---

### 编译指南

1.  进入项目根目录，创建并进入 `build` 目录。

    ```bash
    mkdir build && cd build
    ```

2.  使用 CMake 生成 Makefile。

    ```bash
    cmake ..
    ```

3.  执行 `make` 进行编译。

    ```bash
    make
    ```

    编译成功后，可执行文件将生成在 `ChatServer/bin` 目录下，包括 `ChatServer` 和 `ChatClient`。

---

### 部署与运行

#### 1. 数据库设置

请确保您的 MySQL 服务器正在运行，并创建一个名为 `chat` 的数据库。然后执行以下 SQL 语句创建所需的表：

```sql
CREATE DATABASE IF NOT EXISTS chat;
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

-- 离线消息表
CREATE TABLE `offlinemessage` (
  `userid` INT NOT NULL,
  `message` VARCHAR(500) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 群组表
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
```
**注意**: 请根据实际情况修改 `src/server/db/db.cpp` 中的数据库连接信息（IP, 用户名, 密码）。

#### 2. 运行服务器

*   **单机模式**:
    直接启动一个服务器实例。
    ```bash
    ./bin/ChatServer 127.0.0.1 6000
    ```

*   **集群模式**:
    1.  **启动多个 ChatServer 实例**，监听在不同的端口上。
        ```bash
        # 终端1
        ./bin/ChatServer 127.0.0.1 8001
        # 终端2
        ./bin/ChatServer 127.0.0.1 8002
        # 终端3
        ./bin/ChatServer 127.0.0.1 8003
        ```

    2.  **配置 Nginx** 进行 TCP 负载均衡。
        修改 `nginx.conf` 文件，添加 `stream` 模块配置。**请注意，`stream` 模块与 `http` 模块是同级的。**

        ```nginx
        # nginx.conf

        # ... http 模块等配置 ...

        stream {
            upstream chat_servers {
                # 默认是轮询策略
                # least_conn; # 也可以使用最少连接策略
                server 127.0.0.1:8001;
                server 127.0.0.1:8002;
                server 127.0.0.1:8003;
            }

            server {
                listen 7000; # Nginx 监听的端口，客户端将连接此端口
                proxy_connect_timeout 5s;
                proxy_timeout 10m; # 长连接超时时间
                proxy_pass chat_servers;
            }
        }
        ```

    3.  **启动/重载 Nginx**。
        ```bash
        sudo nginx -s reload
        ```

#### 3. 运行客户端

客户端连接 Nginx 监听的端口（例如，上述配置中的 7000）。

```bash
./bin/ChatClient 127.0.0.1 7000
```

---

### 客户端命令

客户端启动后，您可以根据提示进行操作。登录成功后，支持以下命令：

| 命令          | 格式                               | 说明                  |
|---------------|------------------------------------|-----------------------|
| `help`        | `help`                             | 显示所有支持的命令    |
| `chat`        | `chat:friendid:message`            | 和好友一对一聊天      |
| `addfriend`   | `addfriend:friendid`               | 添加好友              |
| `creategroup` | `creategroup:groupname:groupdesc`  | 创建群组              |
| `addgroup`    | `addgroup:groupid`                 | 加入群组              |
| `groupchat`   | `groupchat:groupid:message`        | 在群组中聊天          |
| `loginout`    | `loginout`                         | 注销当前用户          |
