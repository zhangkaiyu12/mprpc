#pragma once
#include "google/protobuf/service.h"
#include <memory>
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>
#include "zookeeperutil.h"

// 框架提供的专门服务发布rpc的类

class RpcProvider
{
public:
    void NotifyService(google::protobuf::Service *service);

    // 启动rpc服务节点，开始提供rpc远程调用服务
    void Run();

private:
    // 组合了eventloop
    muduo::net::EventLoop m_eventLoop;

    //服务类型信息
    struct ServiceInfo
    {
        google::protobuf::Service *m_service;
        std::unordered_map<std::string,const google::protobuf::MethodDescriptor*>m_methodMap;
    };
    //存储注册成功的服务对象和其服务方法的信息
    std::unordered_map<std::string,ServiceInfo>m_serviceMap;

    // 新的socket连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr &);
    //已建立连接的读写回调
    void OnMessage(const muduo::net::TcpConnectionPtr &,muduo::net::Buffer *,muduo::Timestamp);
    //Closure回调操作，用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr &,google::protobuf::Message*);
};