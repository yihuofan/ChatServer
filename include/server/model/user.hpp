#ifndef USER_H
#define USER_H

#include <string>
using namespace std;
/*
1. MySQL 类：底层数据库访问层（DB）
职责：
连接数据库
执行原始 SQL 查询和更新
 特点：
面向数据库，不涉及具体业务
可以被多个 Model 复用
2. User 类：数据模型（Model / ORM）
职责：
映射数据库中的 user 表为 C++ 对象
提供字段访问方法（getter/setter）
 特点：
纯粹的数据结构，不包含任何数据库逻辑
方便在不同模块间传递用户信息
3. UserModel 类：数据访问对象（DAO / Repository）
职责：
将 User 对象保存到数据库
从数据库读取数据并转化为 User 对象
实现业务需要的数据库操作
 特点：
面向业务，是连接 Model 和 DB 的桥梁
每个表对应一个 DAO，便于管理和维护
三、这种设计的好处
解耦清晰
各层之间职责明确，互不干扰
易于维护
修改某个功能只需改动对应的类
便于扩展
增加新表只需要添加新的 Model 和 DAO
利于测试
提高复用性
*/

// User表的ORM类
class User
{
public:
    User(int id = -1, string name = "", string pwd = "", string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }

    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setPwd(string pwd) { this->password = pwd; }
    void setState(string state) { this->state = state; }

    int getId() { return this->id; }
    string getName() { return this->name; }
    string getPwd() { return this->password; }
    string getState() { return this->state; }

protected:
    int id;
    string name;
    string password;
    string state;
};

#endif