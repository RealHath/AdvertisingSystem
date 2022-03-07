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
#include "mysql.pb.h"
#include "user.h"

using namespace std;
using namespace ad_namespace;

// typedef std::shared_ptr<ad_namespace::ADUser> user_ptr;
// typedef std::shared_ptr<ad_namespace::Advertise> ad_ptr;
// typedef std::unordered_map<uint64_t, ad_ptr> ad_list;
// extern std::unordered_map<std::string, user_ptr> g_userMap; // 用户映射表
// extern std::unordered_map<std::string, ad_list> g_AUMap;    // 用户广告映射表

extern std::shared_ptr<ad_namespace::ADUser> getUser(string uuid);
extern std::shared_ptr<ad_namespace::ADUser> getAdList(string uuid);
extern std::shared_ptr<ad_namespace::Advertise> getAdvertise(string uuid, uint64_t id);
extern bool initUser(string uuid); // 将用户数据加载到内存

//----------------------------------------------------------
int Ad::login(ad_proto::LoginReq &req, ad_proto::LoginResp &resp)
{
    // 1. 处理入参
    string username = req.username();
    string password = req.password();
    char s[255];
    sprintf(s, "SELECT * FROM user WHERE username='%s' and password='%s';",
            username.c_str(), password.c_str());
    string sql(s);

    // 先查库有没有数据
    auto ret = MyDB::getInstance()->execSQL(sql);

    if (ret.size() <= 0)
    {
        resp.set_err(errorEnum::NO_ACCOUNT);
        resp.set_msg("没有此账号或密码错误");
        return errorEnum::NO_ACCOUNT;
    }
    else
    {
        string uuid = ret[0]["uuid"];
        if (getUser(uuid) == nullptr)
        {
            initUser(uuid);
        }

        resp.set_err(errorEnum::SUCCESS);
        resp.set_msg("登陆成功");
        resp.set_uuid(uuid);
        return errorEnum::SUCCESS;
    }
    return 0;
}
int Ad::regist(ad_proto::RegisterReq &req, ad_proto::RegisterResp &resp)
{
    // 1. 处理入参
    string username = req.username();
    string password = req.password();
    uint64_t phone = req.phone();
    char s[255];
    sprintf(s, "SELECT * FROM user WHERE username='%s' and password='%s';",
            username.c_str(), password.c_str());
    string sql(s);

    // 先查库有没有数据
    auto ret = MyDB::getInstance()->execSQL(sql);

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