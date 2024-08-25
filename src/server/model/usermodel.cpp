#include "usermodel.hpp"
#include "db.hpp"
#include <iostream>
using namespace std;

// user 表的增加方法
bool UserModel::insert(User &user)
{
    // 1 组装sql 语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name , password,state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPassword().c_str(), user.getState().c_str());

    MySQL mysql;
    // 连接数据库
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户ID
            user.setId(mysql_insert_id(mysql.getConnect()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    User user;
    user.setId(-1);
    // 1组装sql 语句
    char sql[1024] = {0};
    sprintf(sql, "select * from user where id = %d", id);
    MySQL mysql;
    if (mysql.connect())
    {                                      // 先连接数据库
        MYSQL_RES *res = mysql.query(sql); // 进行查询操作 使用资源要进行释放
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res); // 通过id去取数据
            if (row != nullptr)
            {
                // 取出来默认都是字符串Id 是int 类型要进行类型转换
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPassword(row[2]);
                user.setState(row[3]);
                // 释放资源
                mysql_free_result(res);
                return user;
            }
        }
    }
    // 如果失败user的id 就是-1 可以通过-1 进行判断登录是否成功
    return user;
}

bool UserModel::updateState(User &user)

{
    // 1 组装sql 语句
    char sql[1024] = {0};
    sprintf(sql, "update user set state='%s' where id ='%d'", user.getState().c_str(), user.getId());
    cout << "sql : " << sql << endl;
    MySQL mysql;
    // 连接数据库
    if (mysql.connect()) // 出作用域析构就会释放资源，可以不必担心
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}


//重置用户的在线状态
void UserModel::resetState()
{
    char sql[1024] ="update user set state='offline' where state='online'";
    MySQL mysql;
     if (mysql.connect()) // 出作用域析构就会释放资源，可以不必担心
    {
       mysql.update(sql);
    }
}