#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

/*
userservice原本是一个本地服务，提供了两个进程内的本地方法，longin和getfriendlists
*/

class UserService : public fixbug::UserServiceRpc // 使用在rpc服务发布端（rpc提供者）
{
public:
    // 本地登录业务
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service:login" << std::endl;
        std::cout << "name:" << name << "pwd:" << pwd << std::endl;
        return true;
    }

    // 本地注册业务
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service:Register" << std::endl;
        std::cout << "id:" << id << "name:" << name << "pwd:" << pwd << std::endl;
        return true;
    }
    // 重写基类UserServiceRpc的虚函数 下面这些方法都是框架直接调用的
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        // 框架给业务上报了请求参数LoginRequest,应用获取相应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 本地业务
        bool login_result = Login(name, pwd);

        // 把响应返回 包括错误码、错误消息、返回值
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        // 执行回调操作 执行响应对象数据的序列化和网络发送(都是由框架来完成的)
        done->Run();
    }

    void Register(::google::protobuf::RpcController *controller,
                  const ::fixbug::RegisterRequest *request,
                  ::fixbug::RegisterResponse *response,
                  ::google::protobuf::Closure *done)
    {
        uint32_t id=request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret=Register(id,name,pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        done->Run();
    }
};

int main(int argc, char **argv)
{
    // 调用框架初始化
    MprpcApplication::Init(argc, argv);

    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动rpc服务发布点，run以后等待rpc1调用
    provider.Run();

    return 0;
}