#include "chatservice.hpp"
#include "public.hpp"
#include "usermodel.hpp"
#include <muduo/base/Logging.h>
#include <vector>
using namespace std;
using namespace muduo;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
}

// 获取消息对应处理器
MsgHandler ChatService::getHandler(int msgid)
{   
    //错误日志
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        //返回默认处理器。空操作
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp)
        {
            LOG_ERROR << "msgid:" << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
    
}

// 处理登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>(); // 获取用户id
    string pwd = js["password"];

    User user = UserModel().query(id); // 根据id查询用户信息
    json response;
    
    if (user.getId() == -1) // 用户不存在
    {
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1; // 用户不存在
        response["errmsg"] = "User not found";
        conn->send(response.dump());
    }
    else if (user.getPwd() != pwd) // 密码错误
    {
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 2; // 密码错误
        response["errmsg"] = "Password error";
        conn->send(response.dump());
    }
    else // 登录成功
    {
        if (user.getState() == "online") // 已经在线
        {
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 3; // 已经在线
            response["errmsg"] = "User already online";
            conn->send(response.dump());
        }
        else // 登录成功，更新状态为在线
        {
            {
                lock_guard<mutex> lock(_connMutex); // 上锁，保护_userConnMap
                _userConnMap.insert({id, conn});    // 将用户id和连接信息存储到_map中
            }
            // 数据库的线程安全由mysql服务器保证，局部变量user等都有自己的线程栈
            user.setState("online"); // 设置用户状态为在线
            // 更新用户状态到数据库
            UserModel().updateState(user);
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0; // 成功
            response["id"] = user.getId(); // 返回用户id
            response["name"] = user.getName(); // 返回用户名
            // 查询是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty()) // 有离线消息
            {       
                response["offline_msgs"] = vec; // 返回离线消息
                // 删除用户的离线消息
                _offlineMsgModel.remove(id);
            }
            conn->send(response.dump());
        }
    }
}

// 处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = UserModel().insert(user); // 创建一个临时对象
    if (state)
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0; // 成功
        response["id"] = user.getId(); // 返回新用户的id
        conn->send(response.dump());
    }
    else
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;         // 失败
        conn->send(response.dump());
    }
}
//客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    // 处理客户端异常退出
    {
        lock_guard<mutex> lock(_connMutex); // 上锁，保护_userConnMap
        for (auto it = _userConnMap.begin(); it != _userConnMap.end();)
        {
            if (it->second == conn) // 找到对应的连接
            {   
                user.setId(it->first); // 获取用户id
                _userConnMap.erase(it);
                break;
            }
            else
            {
                ++it; // 继续遍历
            }
        }
    }
    user.setState("offline");      // 设置用户状态为离线
    UserModel().updateState(user); // 更新用户状态到数据库
    LOG_INFO << conn->name() << " has closed connection.";
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex); // 上锁，保护_userConnMap
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end()) // 找到对应的在线连接
        {
            // 发送消息给目标用户
            it->second->send(js.dump());
            return;
        }
        _offlineMsgModel.insert(toid, js.dump()); // 存储离线消息      
    }
}