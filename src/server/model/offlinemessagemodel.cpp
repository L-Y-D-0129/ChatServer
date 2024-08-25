#include "offlinemessagemodel.hpp"
#include "db.hpp"
#include "user.hpp"

// 存储用户离线消息
void OfflineMsgModel::insert(int userid, string msg)
{
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values('%d','%s')", userid, msg.c_str());
    
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 删除离线消息
void OfflineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid = '%d'", userid);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户的离线消息
vector<string> OfflineMsgModel::query(int userid)
{
    // 1组装sql 语句
    vector<string> vec;
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userid);
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
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
        }
    }

    return vec;
}