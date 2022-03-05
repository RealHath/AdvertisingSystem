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

DEFINE_string(attachment, "", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "127.0.0.1:9000", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");

typedef std::shared_ptr<ad_namespace::ADUser> user_ptr;
typedef vector<ad_namespace::Advertise> ad_list;
std::unordered_map<std::string, user_ptr> g_userMap; // 用户映射表
std::unordered_map<std::string, ad_list> g_adMap;    // 用户广告映射表

namespace ad_namespace
{
    Ad::Ad()
    {
    }
    Ad::~Ad()
    {
    }
    int Ad::costPerClick(ad_proto::CostPerClickReq &req, ad_proto::CostPerClickResp &resp)
    {
        // 1. 处理入参
        string userUuid = req.uuid();
        char s[255];
        sprintf(s, "SELECT * FROM ad WHERE uuid='%s';",
                userUuid.c_str());
        string sql(s);

        // 先查库有没有数据
        auto ret = MyDB::getInstance()->execute(sql);

        if (ret.size() <= 0)
        {
            // 没有再注册
            // string uuid = common::genUUID();
            resp.set_err(errorEnum::NO_AD);
            resp.set_msg("没有此广告");
            return errorEnum::NO_ACCOUNT;
        }
        else
        {
            double t = 0.5;

            char buf[255];
            sprintf(buf, "UPDATE money SET amount=%lf WHERE uuid='%s';",
                    t, userUuid.c_str());
            sql = string(buf);
            MyDB::getInstance()->execute(sql);
            resp.set_err(errorEnum::SUCCESS);
            resp.set_msg("CPC");
            return errorEnum::SUCCESS;
        }
        return 0;
    }
    int Ad::costPerMille(ad_proto::CostPerMilleReq &req, ad_proto::CostPerMilleResp &resp)
    {
        // 1. 处理入参
        string uuid = req.uuid();
        char s[255];
        sprintf(s, "SELECT * FROM ad WHERE uuid='%s';",
                uuid.c_str());
        string sql(s);

        // 先查库有没有数据
        auto ret = MyDB::getInstance()->execute(sql);

        if (ret.size() <= 0)
        {
            // 没有再注册
            // string uuid = common::genUUID();
            resp.set_err(errorEnum::NO_AD);
            resp.set_msg("没有此广告");
            return errorEnum::NO_ACCOUNT;
        }
        else
        {
            double t = 0.5;

            char buf[255];
            sprintf(buf, "UPDATE money SET amount=%lf WHERE uuid='%s';",
                    t, uuid.c_str());
            sql = string(buf);
            MyDB::getInstance()->execute(sql);
            resp.set_err(errorEnum::SUCCESS);
            resp.set_msg("CPM");
            return errorEnum::SUCCESS;
        }
        return 0;
    }
    int Ad::costPerAction(ad_proto::CostPerActionReq &req, ad_proto::CostPerActionResp &resp)
    {
        // 1. 处理入参
        string uuid = req.uuid();
        char s[255];
        sprintf(s, "SELECT * FROM ad WHERE uuid='%s';",
                uuid.c_str());
        string sql(s);

        // 先查库有没有数据
        auto ret = MyDB::getInstance()->execute(sql);

        if (ret.size() <= 0)
        {
            // 没有再注册
            // string uuid = common::genUUID();
            resp.set_err(errorEnum::NO_AD);
            resp.set_msg("没有此广告");
            return errorEnum::NO_ACCOUNT;
        }
        else
        {
            double t = 0.5;

            char buf[255];
            sprintf(buf, "UPDATE money SET amount=%lf WHERE uuid='%s';",
                    t, uuid.c_str());
            sql = string(buf);
            MyDB::getInstance()->execute(sql);
            resp.set_err(errorEnum::SUCCESS);
            resp.set_msg("CPA");
            return errorEnum::SUCCESS;
        }
        return 0;
    }
    int Ad::putAdvertise(ad_proto::PutAdvertiseReq &req, ad_proto::PutAdvertiseResp &resp)
    {
        // 1. 处理入参
        string uuid = req.uuid();
        char s[255];
        sprintf(s, "SELECT * FROM ad WHERE uuid='%s';",
                uuid.c_str());
        string sql(s);

        // 先查库有没有数据
        auto ret = MyDB::getInstance()->execute(sql);

        if (ret.size() <= 0)
        {
            // 没有再注册
            // string uuid = common::genUUID();
            resp.set_err(errorEnum::NO_AD);
            resp.set_msg("没有此广告");
            return errorEnum::NO_ACCOUNT;
        }
        else
        {
            double t = 0.5;

            char buf[255];
            sprintf(buf, "UPDATE money SET amount=%lf WHERE uuid='%s';",
                    t, uuid.c_str());
            sql = string(buf);
            MyDB::getInstance()->execute(sql);
            resp.set_err(errorEnum::SUCCESS);
            resp.set_msg("CPA");
            return errorEnum::SUCCESS;
        }
        return 0;
    }

    int Ad::getAdInfo(ad_proto::GetAdInfoReq &req, ad_proto::GetAdInfoResp &resp)
    {
        // 1. 处理入参
        string uuid = req.uuid();
        char s[255];
        sprintf(s, "SELECT * FROM ad WHERE uuid='%s';",
                uuid.c_str());
        string sql(s);

        // 先查库有没有数据
        auto ret = MyDB::getInstance()->execute(sql);

        if (ret.size() <= 0)
        {
            // 没有再注册
            // string uuid = common::genUUID();
            resp.set_err(errorEnum::NO_AD);
            resp.set_msg("没有此广告");
            return errorEnum::NO_ACCOUNT;
        }
        else
        {
            double t = 0.5;

            char buf[255];
            sprintf(buf, "UPDATE money SET amount=%lf WHERE uuid='%s';",
                    t, uuid.c_str());
            sql = string(buf);
            MyDB::getInstance()->execute(sql);
            resp.set_err(errorEnum::SUCCESS);
            resp.set_msg("CPA");
            return errorEnum::SUCCESS;
        }
        return 0;
    }

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
        auto ret = MyDB::getInstance()->execute(sql);

        if (ret.size() <= 0)
        {
            // 没有再注册
            // string uuid = common::genUUID();
            resp.set_err(errorEnum::NO_ACCOUNT);
            resp.set_msg("没有此账号");
            return errorEnum::NO_ACCOUNT;
        }
        else
        {
            string uuid = ret[0]["uuid"];
            if (getUser(uuid) == nullptr)
            {
                uint64_t phone = strtoul(ret[0]["phone"].c_str(), nullptr, 0);
                char buf[255];
                sprintf(buf, "SELECT * FROM money WHERE uuid='%s';",
                        uuid.c_str());
                string sql(buf);
                ret = MyDB::getInstance()->execute(sql);
                double amount = strtod(ret[0]["amount"].c_str(), nullptr);
                double welfare = strtod(ret[0]["welfare"].c_str(), nullptr);
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
        auto ret = MyDB::getInstance()->execute(sql);

        string uuid;

        if (ret.size() <= 0)
        {
            // 没有再注册
            uuid = common::genUUID();
            resp.set_err(errorEnum::SUCCESS);
            resp.set_msg("注册成功");

            // 注册
            char buf1[255];
            sprintf(buf1, "INSERT INTO user(id,username,password,phone,uuid,registTime) VALUES(null,'%s','%s',%lu,'%s',%ld);",
                    username.c_str(), password.c_str(), phone, uuid.c_str(), time(NULL));

            sql = string(buf1);
            MyDB::getInstance()->execute(sql);

            //资金
            char buf2[255];
            sprintf(buf2, "INSERT INTO money(id,uuid,amount,welfare,updateTime) VALUES(null,'%s',%lf,%lf,%ld);",
                    uuid.c_str(), 0.0, 0.0, time(NULL));

            sql = string(buf2);
            MyDB::getInstance()->execute(sql);

            std::unordered_map<string, string> tmp;
            tmp["uuid"] = uuid;
            tmp["username"] = username;
            tmp["phone"] = to_string(phone);
            tmp["amount"] = to_string(0.0);
            tmp["welfare"] = to_string(0.0);
            reflectUser(tmp);
        }
        else
        {
            uuid = ret[0]["uuid"];
            if (getUser(uuid) == nullptr)
            {
                char s1[255];
                sprintf(s1, "SELECT * FROM money WHERE uuid='%s';",
                        uuid.c_str());
                string sql(s);
                auto ret1 = MyDB::getInstance()->execute(sql);

                //映射到内存
                std::unordered_map<string, string> tmp;
                tmp["uuid"] = ret[0]["uuid"];
                tmp["username"] = ret[0]["username"];
                tmp["phone"] = ret[0]["phone"];
                tmp["amount"] = ret1[0]["amount"];
                tmp["welfare"] = ret1[0]["welfare"];
                reflectUser(tmp);
            }
            resp.set_err(errorEnum::HASBEEN_REGISTER);
            resp.set_msg("已注册");
        }
        resp.set_uuid(uuid);
        return 0;
    }

    /////////---------------------------------------------------
    int Ad::getFundInfo(ad_proto::GetFundInfoReq &req, ad_proto::GetFundInfoResp &resp)
    {
        // 1. 处理入参
        string uuid = req.uuid();

        char s[255];
        sprintf(s, "SELECT * FROM money WHERE uuid='%s';",
                uuid.c_str());
        string sql(s);

        // 先查库有没有数据
        auto ret = MyDB::getInstance()->execute(sql);

        if (ret.size() <= 0)
        {
            // 初始化资金
            char buf[255];
            sprintf(buf, "INSERT INTO money(id,uuid,amount,welfare) VALUES(null,'%s',%lf,%lf);",
                    uuid.c_str(), 0.0, 0.0);

            sql = string(buf);
            MyDB::getInstance()->execute(sql);

            resp.set_err(errorEnum::SUCCESS);
            resp.set_msg("查找成功");
            resp.set_uuid(uuid);
            resp.set_amount(0);
            resp.set_welfare(0);
        }
        else
        {
            // auto tmp = ret[0];
            // string uuid = tmp[1];
            // double amount = strtod(tmp[2].c_str(), nullptr);
            // double welfare = strtod(tmp[3].c_str(), nullptr);

            // resp.set_err(errorEnum::SUCCESS);
            // resp.set_msg("查找成功");
            // resp.set_uuid(uuid);
            // resp.set_amount(amount);
            // resp.set_welfare(welfare);
            // return errorEnum::SUCCESS;
        }
        return 0;
    }
    int Ad::recharge(ad_proto::RechargeReq &req, ad_proto::RechargeResp &resp)
    {
        // 1. 处理入参
        string uuid = req.uuid();
        double money = req.money();

        char s[255];
        sprintf(s, "SELECT * FROM money WHERE uuid='%s';",
                uuid.c_str());
        string sql(s);

        // 先查库有没有数据
        auto ret = MyDB::getInstance()->execute(sql);

        if (ret.size() <= 0)
        {
            // resp.set_err(errorEnum::MYSQL_QUERY_ERR);
            // resp.set_msg("查找失败");
            char buf[255];
            sprintf(buf, "INSERT INTO money(id,uuid,amount,welfare) VALUES(null,'%s',%lf,0.0);",
                    uuid.c_str(), money);

            sql = string(buf);
            MyDB::getInstance()->execute(sql);

            resp.set_err(errorEnum::SUCCESS);
            resp.set_msg("充值成功");
            resp.set_uuid(uuid);
            resp.set_amount(money);
            resp.set_welfare(0);
            return errorEnum::MYSQL_QUERY_ERR;
        }
        else
        {
            // auto tmp = ret[0];
            // string uuid = tmp[1];
            // double amount = strtod(tmp[2].c_str(), nullptr);
            // double welfare = strtod(tmp[3].c_str(), nullptr);
            // amount += money;

            // char buf[255];
            // sprintf(buf, "UPDATE money SET amount=%lf WHERE uuid='%s';",
            //         amount, uuid.c_str());

            // sql = string(buf);
            // MyDB::getInstance()->execute(sql);

            // resp.set_err(errorEnum::SUCCESS);
            // resp.set_msg("充值成功");
            // resp.set_uuid(uuid);
            // resp.set_amount(amount);
            // resp.set_welfare(welfare);
            // return errorEnum::SUCCESS;
        }
        return 0;
    }
    int Ad::deduction(ad_proto::DeductionReq &req, ad_proto::DeductionResp &resp)
    {
        // 1. 处理入参
        string uuid = req.uuid();
        double money = req.money();

        char s[255];
        sprintf(s, "SELECT * FROM money WHERE uuid='%s';",
                uuid.c_str());
        string sql(s);

        // 先查库有没有数据
        auto ret = MyDB::getInstance()->execute(sql);

        if (ret.size() <= 0)
        {
            resp.set_err(errorEnum::MYSQL_QUERY_ERR);
            resp.set_msg("查找失败");
            return errorEnum::MYSQL_QUERY_ERR;
        }
        else
        {
            // auto tmp = ret[0];
            // string uuid = tmp[1];
            // double amount = strtod(tmp[2].c_str(), nullptr);
            // double welfare = strtod(tmp[3].c_str(), nullptr);
            // amount -= money;

            // char buf[255];
            // sprintf(buf, "UPDATE money SET amount=%lf WHERE uuid='%s';",
            //         amount, uuid.c_str());

            // sql = string(buf);
            // MyDB::getInstance()->execute(sql);

            // resp.set_err(errorEnum::SUCCESS);
            // resp.set_msg("扣费成功");
            // resp.set_uuid(uuid);
            // resp.set_amount(amount);
            // resp.set_welfare(welfare);
            // return errorEnum::SUCCESS;
        }
        return 0;
    }

    void Ad::reflectUser(std::unordered_map<string, string> &data)
    {
        if (g_userMap.find(data["uuid"]) != g_userMap.end())
        {
            return;
        }

        string uuid = data["uuid"];
        string username = data["username"];
        uint64_t phone = strtoul(data["phone"].c_str(), nullptr, 0);
        double amount = strtod(data["amount"].c_str(), nullptr);
        double welfare = strtod(data["welfare"].c_str(), nullptr);

        ADUser user(uuid, username, phone, amount, welfare);
        user_ptr ptr = make_shared<ADUser>(user);
        g_userMap[uuid] = ptr;
    }

    user_ptr Ad::getUser(string uuid)
    {
        if (g_userMap.find(uuid) != g_userMap.end())
        {
            return g_userMap[uuid];
        }
        return nullptr;
    }
}