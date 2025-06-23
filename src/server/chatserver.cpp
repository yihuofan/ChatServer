#include "chatserver.hpp"
#include <iostream>
#include <functional>
#include <string>
#include"json.hpp"
#include"chatservice.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

// 初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注册链接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(4);
}

// 启动服务
void ChatServer::start()
{
    _server.start();
}

// 上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if(!conn->connected())//
    {
        ChatService::instance()->clientCloseException(conn); // 业务模块处理异常
        conn->shutdown();
    }
    else
    {
        cout << "ChatServer - " << conn->name() << " has connected." << endl;
    }
    
}

// 上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
   string buf= buffer->retrieveAllAsString();
   // 解析json数据
   json js = json::parse(buf);
   //解耦合网络模块和业务模块代码
   //通过js["msgid"]获取=》handler所需参数
   auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
   //回调时间处理器
   msgHandler(conn, js, time);
}