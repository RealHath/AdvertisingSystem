#ifndef __LOGIN_H__
#define __LOGIN_H__
#include <iostream>
#include <string>
#include <memory>
#include "login.pb.h"
#include "mysql.pb.h"

using namespace std;

namespace login_namespace
{
    class Login
    {
    public:
        /* data */
        string username;
        string password;
        string uuid;
        int64_t phone;

    public:
        Login(/* args */);
        ~Login();
        void invoke(mysql_proto::SaveReq &request, mysql_proto::SaveResp &response);
        int login(login_proto::LoginReq &req, login_proto::LoginResp &resp);        //登录逻辑
        int regist(login_proto::RegisterReq &req, login_proto::RegisterResp &resp); //注册逻辑
    };
}

#endif