#include "json.hpp"
using json = nlohmann::json;

#include<iostream>
#include<vector>
#include<map>
using namespace std;


//序列化以后的字符串
string func(){ // 可以理解成 无序hash表
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello , what are you doing?";
    //利用dump 方法就可以将json 序列化以后的字符串赋值给string 
    string sendBuf = js.dump();
    // cout<<"sendBuf : "<<sendBuf<<endl;
    // cout<<"json : "<<js<<endl;
    return sendBuf;
}

//json 示例化 2 稍微复杂类型的定义
string func1(){
    json js ;  //----> 可以看作 map 表 放键值对的
    //添加数组
    js["id"] = {1,2,3,4,5};
    //正常的key -> value
    js["name"] = "zhang san";
    //给msg 的值 在服一个值 相当于映射 
    //值本身也可以是一个json 对象类型

    //js["msg"]["zhang san"] ="hello world";
   // js["msg"]["liu yu ding"] ="hello china!";

    js["msg"] = {{"zhang san","hello world"},{"liu yu ding ","hello china!"}};
    cout<<js<<endl;

    return js.dump();
}

//将STL 直接转化成json 
string func2(){

    json js;
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);
    js["list"] = vec;

    map<int,string> mp;
    mp.insert({1,"泰山"});
    mp.insert({2,"黄山"});
    mp.insert({3,"华山"});
    js["path"] = mp;
    //同样可以将 json -> 序列化成 json字符串进行发送！
    string sendBuf = js.dump();
    cout<<sendBuf<<endl;
    cout<<"------------------------"<<endl;
    cout<<js<<endl;
    return js.dump();
}

//序列化过程  是将数据对象 --> 转化成 json 字符串
void test01(){
    func2();
    func1();
    func();
}
int main(){
    //反序列化 是将 json字符串 --> 转成数据对象
    string sendBuf = func2();
    //parse 解析函数从字符串中解析出json 对象
    json js = json::parse(sendBuf);

    // auto arr = js["id"];
    // for(int i =0;i<arr.size();i++){
    //     cout<<arr[i]<<endl;
    // }
    // auto msgjs = js["msg"];
    // cout<<msgjs["zhang san"]<<endl;
    //cout<<js["msg_type"]<<endl;

    // vector<int> vec = js["list"];
    // for(auto &it:vec){
    //     cout<<it<<" ";
    // }
    // cout<<endl;

    // map<int,string> mp = js["path"];
    // for(auto &p:mp){
    //     cout<<p.first<<" "<<p.second<<endl;
    // }
    // cout<<endl;
    return 0;
}
