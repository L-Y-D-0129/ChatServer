#include "friendmodel.hpp"
#include "db.hpp"

// 添加好友
void FriendModel::insert(int userid, int friendid)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values('%d','%d')", userid, friendid);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 显示返回好友列表
vector<User> FriendModel::qurey(int userid)
{
    // 1组装sql 语句
    vector<User> vec;
    char sql[1024] = {0};
    //做一个内连接的联合查询 
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where b.userid='%d';", userid);
    MySQL mysql;
    if (mysql.connect())
    {                                      // 先连接数据库
        MYSQL_RES *res = mysql.query(sql); // 进行查询操作 使用资源要进行释放
        if (res != nullptr)
        {
            MYSQL_ROW row; // 通过id去取数据
            // 把 userid用户的所有消息放入res 中返回
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
        }
    }

    return vec;
}
