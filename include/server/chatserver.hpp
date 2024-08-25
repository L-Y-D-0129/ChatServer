#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;

//聊天服务器的主类
class ChatServer
{

public:
    // 初始化Tcp服务器对象
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);
    // 开始服务。
    void start();

private:
    // 上报连接断开信息的回调函数
    void onConnection(const TcpConnectionPtr &conn);

    // 上报读写事件相关信息的回调函数
    void onMessage(const TcpConnectionPtr & conn,
                   Buffer *,
                   Timestamp);
    TcpServer _server; //组合muduo库，实现服务器功能的对象
    EventLoop *_loop; // 事件循环对象的指针 --》在合适的时候来执行 quit
}; //需要; 
#endif