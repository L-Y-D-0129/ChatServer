/*
muduo 网络库给用户提供了两个主要的类
TcpServer : 用于编写服务器程序的
TcpClient : 用于编写客户端程序的

epoll + 线程池
好处 ： 能够把网络I/O的代码和业务代码区分开 （最重要的） 我们关注的是业务代码 就是一下两点
    我们只需要考虑这两件事：
                        用户的连接和断开  用户的可读写事件

*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional> //用于bind 的头文件
#include <string>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders; //? 设置占位符

/* 基于muduo 网络库开发服务程序
    1. 组合TcpServer 对象
    2. 创建EventLoop 事件循环对象的指针 --> epoll
    3. 明确TcpServer 构造函数需要什么参数，输出ChatServer 的构造函数
    4. 在当前服务器类的构造函数当中，注册处理连接的回调函数和读写事件的回调函数
    5. 设置合适的服务端线程书数量，muduo库会自己分配 I/O 线程 和 worker 线程
*/
class ChatServer
{

public:
    ChatServer(EventLoop *loop,               // 事件循环
               const InetAddress &listenAddr, // IP + Port
               const string &nameArg)         // 服务器的名字
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        // 给服务器注册用户连接的创建和断开回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,_1));

        // 给服务器注册用户读写事件回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

        // 设置服务器端的线程数量 1 个 I/O线程 3 个worker线程
        _server.setThreadNum(4);
    }
    // 开启事件循环
    void start()
    {
        _server.start();
    }

private:
    // *专门处理用户连接创建和断开 epoll listenfd accept 
    //类型是一个智能指针
    void onConnection( const TcpConnectionPtr &conn)
    {

        if (conn->connected())
        {
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() <<"state: online"<< endl;
        }else {
            cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() <<"state:  off online"<< endl; 
            conn->shutdown();//close(fd)
            //_loop->quit() 
        }
    }

    // *专门处理用户的读写事件的
    void onMessage(const TcpConnectionPtr &conn, // 连接
                   Buffer *buffer,                  // 缓冲区 从buf 中读数据
                   Timestamp time)                    // 接收到数据的时间信息
    {
        string buf = buffer->retrieveAllAsString(); //获取所有数据
        cout<<"recv data: "<<buf <<"time "<<time.toString()<<endl;
        conn->send(buf);
    }

    
    TcpServer _server; // #1  --> 没有默认构造就要在ChatServer的构造函数中给 _server 构造
    EventLoop *_loop;  // #2 用于关闭服务器程序 有事件发生loop会给我们上报的
};


int main(){
    EventLoop loop; // epoll
    InetAddress addr("192.168.239.128",6000);
    ChatServer server (&loop,addr,"ChatServer");

    server.start(); //启动我们的服务器 listenfd epoll_ctl -> epoll 
    loop.loop(); //epoll_wait 以阻塞的方式等待新用户连接 已连接用户的读写事件等待
    return 0;
}