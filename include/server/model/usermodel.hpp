#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"
// User表的数据操作类
class UserModel
{

public:
    // User表的增加方法
    bool insert(User &user);

    // 查询方法
    User query(int id);

    // 更新用户的状态信息
    bool updateState(User &user);

    //更新用户的在线状态
    void resetState();
};

#endif