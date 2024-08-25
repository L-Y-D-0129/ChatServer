#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>

// 获取单例接口  ---- 业务层操作都是对象 ------ DAO 就是数据层
ChatService *ChatService::getinstance()
{
    static ChatService chatserver;
    return &chatserver;
}

// 注册消息以及对应的Handler处理
ChatService::ChatService()
{
    // 绑定起和回调的过程
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::onechat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    // 群组业务管理相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});

    if (_redis.connect())
    {
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,_1,_2));
    }
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msgid 没有对应的消息处理器
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            // 使用muduo日志的错误
            LOG_ERROR << "msgid :" << msgid << " can not find handler!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

UserModel ChatService::getUserModel()
{
    return _usermodel;
}

// 注册 网络模块将消息反序列化 -- 生成json的数据对象，上报到注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{ // name password
    string name = js["name"];
    string password = js["password"];
    // 操作数据对象
    User user;
    user.setName(name);
    user.setPassword(password);
    // 数据库操作类进行注册操作
    bool state = _usermodel.insert(user);

    if (state)
    {
        // 成功！
        json resjs;
        resjs["msgid"] = REG_MSG_ACK;
        resjs["errno"] = 0; // 如果是 0 就代表成功了
        resjs["id"] = user.getId();
        conn->send(resjs.dump()); // 给客户端发送回去
        LOG_INFO << "create user successful";
    }
    else
    {
        // 失败
        json resjs;
        resjs["msgid"] = REG_MSG_ACK;
        resjs["errno"] = 1;       // 如果是 1 就代表失败了
        conn->send(resjs.dump()); // 给客户端发送回去
        LOG_INFO << "create user fail";
    }
}

// 登录 msgid id password
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"];
    string password = js["password"];
    // 在用户表中查询id
    User user = _usermodel.query(id);

    if (user.getId() != -1 && user.getPassword() == password)
    {
        if (user.getState() == "online")
        {
            // 失败！用户已在线 ，请勿重复登录
            json resjs;
            resjs["msgid"] = LOGIN_MSG_ACK;
            resjs["errno"] = 2; // 该账户已经登录
            resjs["errmsg"] = "this account is using , input another!";
            conn->send(resjs.dump()); // 给客户端发送回去
            LOG_INFO << "Login  fail";
        }
        else
        {
            // 将登录成功的记录用户连接信息 ----> 涉及到多线程 ---> 要考虑线程安全问题
            // 当大量用户大量一起登录 一起想map 中插入的时候就遇到了线程安全问题 由于C++ 中的
            // STL容器不带有线程安全问题，所以要进行维护线程安全 使用互斥锁来解决这个问题。
            { // 加个作用域的就可使只保证这一部分的安全问题
                // 因为后面的变量都是局部变量 线程之间栈空间使独立的 所以不用进行加锁！
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({user.getId(), conn});
            }

            // id 用户登录成功后 ， 要在 redis 中订阅
            _redis.subscribe(id);

            // 成功！ 要更新用户的在先状态 由 offline -> online
            user.setState("online");

            // 使用数据库操作类 进行刷新
            LOG_INFO << "User state :" << user.getState();
            _usermodel.updateState(user);
            json resjs;
            resjs["msgid"] = LOGIN_MSG_ACK;
            // 如果是 0 就代表成功了
            resjs["errno"] = 0;
            resjs["id"] = user.getId();
            resjs["name"] = user.getName();

            // 查看用户是否有离线消息
            vector<string> vec = _offlienMsgModel.query(user.getId());
            if (!vec.empty())
            {
                resjs["offlinemessage"] = vec;
                _offlienMsgModel.remove(user.getId());
            }
            // 查询该用户的好友信息并返回
            vector<User> userVec = _friendmodel.qurey(id);
            if (!userVec.empty())
            {
                vector<string> vec2;
                for (auto &it : userVec)
                {
                    json js;
                    js["id"] = it.getId();
                    js["name"] = it.getName();
                    js["state"] = it.getState();
                    vec2.push_back(js.dump());
                }
                resjs["friends"] = vec2;
            }
            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupmodel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                vector<string> groupV;
                for (auto &it : groupuserVec)
                {
                    json groupjs;
                    groupjs["id"] = it.getId();
                    groupjs["groupname"] = it.getName();
                    groupjs["groupdesc"] = it.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : it.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    groupjs["users"] = userV;
                    groupV.push_back(groupjs.dump());
                }

                resjs["groups"] = groupV;
            }

            // 给客户端发送回去
            conn->send(resjs.dump());
            LOG_INFO << "Login  successful";
        }
    }
    else
    {
        // 失败！
        json resjs;
        resjs["msgid"] = LOGIN_MSG_ACK;
        resjs["errno"] = 1; // 登录失败
        if (user.getId() == id)
        {
            resjs["errmsg"] = "id or password is invalid!";
        }
        else
        {
            resjs["errmsg"] = "The user does not exist, please register first";
        }

        conn->send(resjs.dump()); // 给客户端发送回去
        LOG_INFO << "Login  fail";
    }
}

void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it); // 从在先用户映射表中删除
        }
    }

    // 用户下线要取消订阅
    _redis.unsubscribe(userid);

    // 更新用户的在线状态为offline
    User user(userid);
    LOG_INFO << "userid"<<user.getId();
    _usermodel.updateState(user);
}

// 客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user; // 这里注意user 要放在外面
    {
        // 异常退出一样有线程安全问题 和 插入同理
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it->second == conn)
            {
                // 1 遍历userConnMap 将异常退出的删除
                _userConnMap.erase(it);
                user.setId(it->first);
                break;
            }
        }
    }

    // 用户下线要取消订阅
    _redis.unsubscribe(user.getId());

    if (user.getId() != -1)
    {
        // 2 跟新用户状态 offline
        user.setState("offline");
        _usermodel.updateState(user);
    }
}

// 一对一聊天业务 msgid id toid msg
void ChatService::onechat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // 在线 进行消息转发 A -> Server -> B  服务器主动推送消息给B用户
            it->second->send(js.dump());
            return;
        }
    }
   
    // 查看toid用户是否在不同的服务器上线
    User user = _usermodel.query(toid);
    LOG_INFO << "userid : "<<js["id"].get<int>();
    LOG_INFO << "toid : "<<toid;
    LOG_INFO << "toUserstate : "<<user.getState();
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }
    LOG_INFO<<"--------------------------1";
    // 不在_userConnMap 并且 也不在其他服务器上线那么才代表 不在线 , 存储离线消息
    _offlienMsgModel.insert(toid, js.dump());
}
// 处理服务器异常;业务重置方法
void ChatService::reset()
{
    _usermodel.resetState();
}

// 添加好友 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 储存好友信息
    _friendmodel.insert(userid, friendid);
}

// 创建群组业务  id groupname groupdesc
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 储存新创建群组的信息
    Group group(-1, name, desc);
    if (_groupmodel.createGroup(group))
    {
        // 储存创建人的信息
        _groupmodel.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组 id groupid
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    string role = "normal";
    _groupmodel.addGroup(userid, groupid, role);
}

// 群聊天 id groupid msg
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();

    vector<int> useridVec = _groupmodel.queryGroupUsers(userid, groupid);
    lock_guard<mutex> lock(_connMutex);
    for (auto &id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            // 查看toid用户是否在不同的服务器上线
            User user = _usermodel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                // 储存离线群消息
                _offlienMsgModel.insert(id, js.dump());
            }
            
        }
    }
}

//从redis 消息队列中获取订阅的消息  问题
void ChatService::handleRedisSubscribeMessage(int userid , string message)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    LOG_INFO<<"userid  "<<userid;
    if (it != _userConnMap.end())
    {
        it->second->send(message);
        return;
    }

    // 存储该用户的离线消息
    _offlienMsgModel.insert(userid, message);

}