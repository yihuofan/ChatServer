/*
muduo库提供两个主要类：
TcpServer：用于处理TCP连接的服务器类。
TcpClient：用于处理TCP连接的客户端类。
*/
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <thread>
#include <string>
using namespace muduo;
using namespace muduo::net;
using namespace std;
using namespace placeholders;
/*基于muduo库的TCP服务器类ChatServer
 * 该类封装了一个事件循环和一个TCP服务器，用于处理聊天功能。
 * 主要功能包括：
 * - 初始化事件循环和TCP服务器
 * - 设置连接回调函数
 * - 启动服务器

*/

class ChatServer
{
public:
    ChatServer(EventLoop *loop,               // 事件循环指针
               const InetAddress &listenAddr, // 监听地址
               const string &nameArg)         // 服务器名称
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 设置连接回调函数
        _server.setConnectionCallback(
            std::bind(&ChatServer::onConnection, this, _1));
        // 设置消息回调函数
        _server.setMessageCallback(
            std::bind(&ChatServer::onMessage, this, _1, _2, _3));
        // 动态设置线程数为CPU核心数
        unsigned int cores = std::thread::hardware_concurrency();
        _server.setThreadNum(cores > 0 ? cores : 4); // 如果获取失败则默认使用4
    }
    void start() // 启动服务器
    {
        _server.start(); // 启动TCP服务器
        _loop->loop();   // 进入事件循环
    }

private:
    // 处理连接的回调函数
    void onConnection(const TcpConnectionPtr &conn)
    {
        if (conn->connected()) // 如果连接已建立
        {
            cout << "New connection: " << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort()
                 << endl; // 输出对端连接信息
        }
        else // 如果连接已断开
        {
            cout << "Connection closed: " << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort()
                 << endl;
            conn->shutdown(); // 关闭连接
        }
    }
    void onMessage(const TcpConnectionPtr &conn, // 连接指针
                   Buffer *buf,                  // 接收缓冲区
                   Timestamp time)               // 时间戳
    {
        string msg = buf->retrieveAllAsString(); // 获取接收到的消息

        // 格式化时间显示
        string timeStr = time.toFormattedString();
        cout << "Received message: " << msg << " at " << timeStr << endl;

        conn->send(msg + "haha"); // 将消息回送给客户端
    }

    EventLoop *_loop;  // 事件循环
    TcpServer _server; // TCP服务器
};

int main()
{
    EventLoop loop;                                     // 创建事件循环
    InetAddress listenAddr(8888);                       // 监听地址，端口号为8888
    ChatServer server(&loop, listenAddr, "ChatServer"); // 创建聊天服务器实例
    server.start();                                     // 启动服务器
    cout << "Chat server started on port 8888" << endl; // 输出服务器启动信息
    // 进入事件循环
    loop.loop(); // 以阻塞方式进入事件循环，等待连接和消息处理
    return 0;    // 程序结束
}