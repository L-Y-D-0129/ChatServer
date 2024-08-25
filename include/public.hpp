#ifndef PUBLIC_H
#define PUBLIC_H

// server 和 client 公用的 
//也就是我们想要的效果通过MsgType 的值去调用不同的方法解决
enum EnMsgType{
    //登录
    LOGIN_MSG =1, //请求
    LOGIN_MSG_ACK,//响应
    //注册
    REG_MSG,
    REG_MSG_ACK,
    //私聊
    ONE_CHAT_MSG,
    ONE_CHAT_MSG_ACK,
    //添加好友
    ADD_FRIEND_MSG,
    //创建群组
    CREATE_GROUP_MSG,
    //加入群组
    ADD_GROUP_MSG,
    //群聊天
    GROUP_CHAT_MSG,
    //等出
    LOGINOUT_MSG
};

#endif