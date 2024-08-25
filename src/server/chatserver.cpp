#include "chatserver.hpp"
#include <json.hpp>
#include <functional>
#include <string>
#include "chatservice.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注册链接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(4);
}

void ChatServer::start()
{
    _server.start();
}

// 监听连接和断开！
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 客户断开连接，释放套接字
    if (!conn->connected())
    {
        ChatService::getinstance()->clientCloseException(conn);
        conn->shutdown();
    }
}

// 给服务层通知的
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    //将消息通过retrieveAllAsString 都读出来
    string buf = buffer->retrieveAllAsString();
    //数据的反序列化 
    json js = json::parse(buf);
    //我们希望做到高内聚，低耦合  这里就要区分开网络模块的代码和业务模块的代码
    auto msgHandler = ChatService::getinstance()->getHandler(js["msgid"].get<int>());
    //通过回调消息绑定好的事件处理器，来执行响应的业务处理。
    msgHandler(conn,js,time);
}