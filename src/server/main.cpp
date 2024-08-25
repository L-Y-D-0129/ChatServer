#include"chatserver.hpp"
#include <signal.h>
#include "chatservice.hpp"
#include<iostream>
using namespace std;

//处理服务器 异常退出的用户在线状态设置，
void resetHandler(int){
    ChatService::getinstance()->reset();
    exit(0);
}
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 192.168.239.128 6000" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}