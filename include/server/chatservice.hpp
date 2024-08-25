#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <functional>
#include <unordered_map>
#include <mutex>
// #include <memory>
#include "redis.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;

// 处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;

// 聊天服务器业务类 --> 实现单例模式 构造函数私有化 删除拷贝构造和赋值运算符
class ChatService
{
public:
    // 获取单例的接口
    static ChatService *getinstance();
    // 处理登录
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理等出
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理注册
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 获取数据库操作对象
    UserModel getUserModel();
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
    // 一对一聊天业务
    void onechat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 处理服务器异常;业务重置方法
    void reset();
    // 处理添加好友的业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //创建群组
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //加入群组
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    //群聊天
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    //从redis 消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int userid , string message);

private:
    ChatService();
    ChatService(const ChatService &instance) = delete;
    ChatService *operator=(const ChatService &) = delete;
    // 储存id 对应的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;
    // 数据库操作对象
    UserModel _usermodel;
    // 使用互斥锁来保证_userConnMap 的线程安全问题
    mutex _connMutex;

    // 数据操作类对象
    // 1.存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    // 2.离线消息处理对象
    OfflineMsgModel _offlienMsgModel;
    // 3.好友表操作对象
    FriendModel _friendmodel;
    // 4.群组操作对象
    GroupModel _groupmodel;

    //redis 操作对象
    Redis _redis;
};
#endif