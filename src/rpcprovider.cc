#include "rpcprovider.h"
#include "mprpcapplication.h"
#include "rpcheader.pb.h"


//框架提供给外部使用，发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    //获取服务信息
    //服务名字
    std::string service_name = pserviceDesc->name();
    //服务对象service的方法数量
    int methodCnt = pserviceDesc->method_count();

    std::cout<<"service_name:"<<service_name<<std::endl;

    for(int i=0;i<methodCnt;i++)
    {
        //获取服务对象指定下标的抽象描述
        const google::protobuf::MethodDescriptor* pmethodDesc= pserviceDesc->method(i);
        std::string method_name=pmethodDesc->name();
        service_info.m_methodMap.insert({method_name,pmethodDesc});
        std::cout<<"method_name:"<<method_name<<std::endl;
    }
    service_info.m_service=service;
    m_serviceMap.insert({service_name,service_info});
}

// 启动rpc服务节点，开始提供rpc远程调用服务
void RpcProvider::Run()
{
    //读取ip地址与端口号port
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    //创建tcpserver对象
    muduo::net::TcpServer server(&m_eventLoop,address,"RpcProvider");
    //绑定连接回调，消息回调
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection,this,std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage,this,std::placeholders::_1
    ,std::placeholders::_2,std::placeholders::_3));

    //设置muduo的线程数量
    server.setThreadNum(4);

    //把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    ZkClient zkCli;
    zkCli.Start();
    //service_name为永久性节点，method为临时性节点
    for (auto &sp:m_serviceMap)
    {
        //service_name
        std::string service_path="/"+sp.first;
        zkCli.Create(service_path.c_str(),nullptr,0);
        for (auto &mp:sp.second.m_methodMap)
        {
            //service_name/method_name   /UserServiceRpc/Login 
            std::string method_path=service_path+"/"+mp.first;
            char method_path_data[128]={0};
            sprintf(method_path_data,"%s:%d",ip.c_str(),port);
            //ZOO_EPHEMERAL临时性节点
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }

    //rpc服务端准备启动，打印消息
    std::cout<<"rpcprovider start service success ip:"<<ip<<"port:"<<port<<std::endl;

    //启动网络服务
    server.start();
    m_eventLoop.loop();
}

//新的socket连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if(!conn->connected())
    {
        //和rpcclient连接断开
        conn->shutdown();
    }
}

/*
在框架内部，rpcprovider与rpcconsumer协商好通信之间用的protobuf数据类型
service_name method_name args 定义proto的message类型，进行数据序列化与反序列化
                                service_name method_name args_size
header_size(4个字节)+header_str+args_str
*/

//已建立连接用户的读写时间回调，如果远程有一个rpc服务调用请求，那么OnMessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,muduo::net::Buffer *buffer,muduo::Timestamp)
{
    //网络上接受远程rpc调用请求的字符流 Login args
    std::string recv_buf = buffer->retrieveAllAsString();

    //从字符流中读取前四个字节的内容
    uint32_t header_size=0;
    recv_buf.copy((char*)&header_size,4,0);

    //根据head_size读取数据头的原始字符流
    std::string rpc_header_str=recv_buf.substr(4,header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        //数据头反序列化成功
        service_name=rpcHeader.service_name();
        method_name=rpcHeader.method_name();
        args_size=rpcHeader.args_size();

    }else{
        //数据头反序列化失败
        std::cout<<"rpc_header_str"<<rpc_header_str<<"parse error!"<<std::endl;
        return;
    }

    //获取rpc方法参数的字符流数据
    std::string args_str=recv_buf.substr(4+header_size,args_size);

    //打印调试信息
    std::cout<<"============================================="<<std::endl;
    std::cout<<"header_size"<<header_size<<std::endl;
    std::cout<<"rpc_header_str"<<rpc_header_str<<std::endl;
    std::cout<<"service_name"<<service_name<<std::endl;
    std::cout<<"method_name"<<method_name<<std::endl;
    std::cout<<"args_str"<<args_str<<std::endl;
    std::cout<<"============================================="<<std::endl;

    //获取service对象和method对象
    auto it= m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout<<service_name<<"is not exist!"<<std::endl;
        return;
    }
    
    auto mit=it->second.m_methodMap.find(method_name);
    if (mit==it->second.m_methodMap.end())
    {
        std::cout<<service_name<<":"<<method_name<<"is not exist!"<<std::endl;
    }

    //获取service对象，new UserService
    google::protobuf::Service *service=it->second.m_service;
    //获取method对象 UserService中的Login
    const google::protobuf::MethodDescriptor* method=mit->second;

    //生成rpc方法调用的请求request和响应参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if  (!request->ParseFromString(args_str))
    {
        std::cout<<"request parse error! content:"<<args_str<<std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    //给下面的method方法的调用，绑定一个Closure的回调函数
    google::protobuf::Closure * done = 
    google::protobuf::NewCallback<RpcProvider,const muduo::net::TcpConnectionPtr&,google::protobuf::Message*>(this,&RpcProvider::SendRpcResponse,conn,response);

    //在框架上根据远端rpc请求，调用当前rpc节点上发布的办法
    service->CallMethod(method,nullptr,request,response,done);
}

//Closure回调操作，用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn,google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {
        conn->send(response_str);
    }
    else{
        std::cout<<"序列化response_str失败"<<std::endl;
    }
    conn->shutdown();
}