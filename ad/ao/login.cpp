#include <butil/logging.h>
#include <gflags/gflags.h>
#include <brpc/channel.h>
#include <time.h>
#include <unordered_map>
#include <memory>

#include "sql.h"
#include "errorEnum.pb.h"
#include "ad.h"
// #include "mysqlSave.h"
#include "common.h"
// #include "mysql.pb.h"
#include "user.h"

using namespace std;
using namespace ad_namespace;

// typedef std::shared_ptr<ad_namespace::ADUser> user_ptr;
// typedef std::shared_ptr<ad_namespace::Advertise> ad_ptr;
// typedef std::unordered_map<uint64_t, ad_ptr> ad_list;
// std::unordered_map<std::string, user_ptr> g_userMap; // 用户映射表
// std::unordered_map<std::string, ad_list> g_AUMap;    // 用户广告映射表

// std::shared_ptr<ad_namespace::ADUser> getUser(string uuid);
// std::shared_ptr<ad_namespace::ADUser> getAdList(string uuid);
// std::shared_ptr<ad_namespace::Advertise> getAdvertise(string uuid, uint64_t id);
// bool initUser(string uuid); // 将用户数据加载到内存

extern std::unordered_map<uint32_t, vector<ad_ptr>> g_typeMap; // 类型广告映射表
extern std::unordered_map<std::string, user_ptr> g_userMap;    // 用户映射表
extern std::unordered_map<uint32_t, ad_ptr> g_adMap;           // 用户广告映射表
extern std::unordered_map<std::string, count_ptr> g_countMap;  // 统计表
extern std::unordered_map<std::string, ad_list> g_AUMap;       // 用户广告映射表

//----------------------------------------------------------
int Ad::login(ad_proto::LoginReq &req, ad_proto::LoginResp &resp)
{
    if (!req.has_username() || !req.has_password())
    {
        resp.set_err(errorEnum::INPUT_ERROR);
        resp.set_msg("输入错误");
        return 0;
    }

    // 1. 处理入参
    string username = req.username();
    string password = req.password();
    char s[255];
    sprintf(s, "SELECT * FROM user WHERE username='%s' and password='%s';",
            username.c_str(), password.c_str());
    string sql(s);

    // 先查库有没有数据
    auto ret = MyDB::getInstance()->syncExecSQL(sql);

    if (ret.size() <= 0)
    {
        resp.set_err(errorEnum::NO_ACCOUNT);
        resp.set_msg("没有此账号或密码错误");
        return errorEnum::NO_ACCOUNT;
    }
    else
    {
        string uuid = ret[0]["uuid"];
        uint64_t phone = strtoull(ret[0]["phone"].c_str(), nullptr, 0);
        if (getUser(uuid) == nullptr)
        {
            initUser(uuid);
        }

        resp.set_err(errorEnum::SUCCESS);
        resp.set_msg("登陆成功");
        resp.set_uuid(uuid);
        resp.set_phone(phone);
        return errorEnum::SUCCESS;
    }
    return 0;
}
int Ad::regist(ad_proto::RegisterReq &req, ad_proto::RegisterResp &resp)
{
    if (!req.has_username() || !req.has_password() || !req.has_phone())
    {
        resp.set_err(errorEnum::INPUT_ERROR);
        resp.set_msg("输入错误");
        return 0;
    }

    // 1. 处理入参
    string username = req.username();
    string password = req.password();
    uint64_t phone = req.phone();

    char s[255];
    sprintf(s, "SELECT * FROM user WHERE username='%s';",
            username.c_str());
    string sql(s);

    // 先查库有没有数据
    auto ret = MyDB::getInstance()->syncExecSQL(sql);

    string uuid;

    if (ret.size() <= 0)
    {
        // 没有再注册
        uuid = common::genUUID();
        resp.set_err(errorEnum::SUCCESS);
        resp.set_msg("注册成功");

        ADUser user(uuid, username, password, phone, 0.0, 0.0);
        user.insertUser();
        g_userMap[uuid] = make_shared<ADUser>(user);
        g_AUMap[uuid] = {};
    }
    else
    {
        uuid = ret[0]["uuid"];
        if (getUser(uuid) == nullptr)
        {
            initUser(uuid);
        }
        resp.set_err(errorEnum::HASBEEN_REGISTER);
        resp.set_msg("已注册");
    }
    resp.set_uuid(uuid);
    return 0;
}