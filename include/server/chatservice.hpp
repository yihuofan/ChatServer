#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
using namespace std;
using namespace muduo;
using namespace muduo::net;


#include "json.hpp"
using json = nlohmann::json;

// 表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

// 聊天服务器业务类
class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService *instance();
    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //获取消息对应处理器
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

private:
    ChatService();

    // 存储消息id和其对应的业务处理方法，在服务器启动时注册，不需要线程安全
    unordered_map<int, MsgHandler> _msgHandlerMap;
    // 存储用户id和对应的会话信息，这个 map 在运行过程中会被多个线程并发地读写
    unordered_map<int, TcpConnectionPtr> _userConnMap; 

    // 互斥锁，保护_userConnMap
    mutex _connMutex;

    // 数据操作对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;   
};

#endif