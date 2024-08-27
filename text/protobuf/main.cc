#include "test.pb.h" 

#include <iostream>
#include <string>
using namespace fixbug;


int main()
{
    // LoginResponse rsp;
    // ResultCode *rc= rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("登录失败");

    GetFriendListsResponse rsp;
    ResultCode *rc=rsp.mutable_result();
    rc->set_errcode(0);

    User *user1=rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(26);
    user1->set_sex(User::MAN);

    User *user2=rsp.add_friend_list();
    user2->set_name("zhang san");
    user2->set_age(26);
    user2->set_sex(User::MAN);

    User *user3=rsp.add_friend_list();
    user3->set_name("zhang san");
    user3->set_age(26);
    user3->set_sex(User::MAN);

    std::cout<< rsp.friend_list_size()<<std::endl;
    User user4= rsp.friend_list(1);
    std::cout<< user4.age()<<user4.name()<<user4.sex()<<std::endl;
    return 0;
}

int main1()
{
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");

    //对象序列化-》 char*
    std::string send_str;
    if (req.SerializeToString(&send_str))
    {
        std::cout<<send_str.c_str()<<std::endl;
    }

    //从send_str反序列化一个login请求对象
    LoginRequest reqb;
    if (reqb.ParseFromString(send_str))
    {
        std::cout<<reqb.name()<<std::endl;
        std::cout<<reqb.pwd()<<std::endl;
    }

    return 0;
}