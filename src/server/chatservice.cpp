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
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    // 群组业务管理相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    // 连接redis服务器
    if (_redis.connect())
    {
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
}

// 服务器异常处理
void ChatService::reset()
{
    // 重置用户状态
    _userModel.resetState(); // 重置所有用户状态为离线
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
            // 订阅用户的redis消息通道(表示这个用户在我这里登陆，所以我关注这个id的消息)
            _redis.subscribe(id);
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
            // 查询用户的好友列表
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty()) // 有好友
            {
                vector<string> friends;
                for (const auto &friendUser : userVec)
                {
                    json friendJson;
                    friendJson["id"] = user.getId();
                    friendJson["name"] = user.getName();
                    friendJson["state"] = user.getState();
                    friends.push_back(friendJson.dump()); // 将好友信息转换为json字符串
                }
                response["friends"] = friends; // 返回好友列表
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

// 处理注销业务
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 取消订阅用户的redis消息通道
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}

//客户端直接退出
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
    int toid = js["to"].get<int>();// 获取目标用户id
    {
        lock_guard<mutex> lock(_connMutex); // 上锁，保护_userConnMap
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end()) // 找到对应的在线连接
        {
            // 发送消息给目标用户
            it->second->send(js.dump());
            return;
        }
           
    }
    // 查询toid用户是否在线，如果不在线，则存储离线消息
    User user = _userModel.query(toid);
    if(user.getState() == "online")
    {
        // 如果用户在线，则直接返回
        _redis.publish(toid, js.dump()); // 发布消息到redis
        return;
    }
    // 如果用户不在线，则存储离线消息
    _offlineMsgModel.insert(toid, js.dump()); // 存储离线消息
}

// 处理添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();
    _friendModel.insert(userid, friendid); // 添加好友关系
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        // 存储群组创建人信息
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())// 在当前服务器中找到用户连接
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            // 查询其他群用户是否在线
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {   
                // 如果在线，则发布消息到redis
                _redis.publish(id, js.dump());
                continue;
            }
            else
            {
                // 存储离线消息
                _offlineMsgModel.insert(id, js.dump());
            }
            
        }
    }
}

// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);
}