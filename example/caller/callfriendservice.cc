#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"

int main(int argc, char **argv)
{
    // 整个程序启动后，想使用mprpc1框架来使用rpc1服务调用，要先初始化
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法Login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());
    // rpc方法请求参数
    fixbug::GetFriendListRequest request;
    request.set_userid(10000);
    // rpc响应
    fixbug::GetFriendListResponse response;
    // rpc控制对象
    MprpcController controller;
    stub.GetFriendList(&controller, &request, &response, nullptr);

    // 一次rpc调用完成，读取调用结果
    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (0 == response.result().errcode())
        {
            std::cout << "rpc GetFriendList response success!" << std::endl;
            int size = response.friends_size();
            for (int i = 0; i < size; i++)
            {
                std::cout << "index:" << (i + 1) << "name:" << response.friends(i) << std::endl;
            }
        }
        else
        {
            std::cout << "rpc GetFriendList response error:" << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}
