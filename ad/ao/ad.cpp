#include <butil/logging.h>
#include <gflags/gflags.h>
#include "ad.h"
// #include "mysqlSave.h"
#include "common.h"
#include "mysql.pb.h"
#include <brpc/channel.h>
#include "errorEnum.pb.h"
#include <time.h>
#include "sql.h"

using namespace std;

DEFINE_string(attachment, "", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "127.0.0.1:9000", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");

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

        return 0;
    }
    int Ad::costPerMille(ad_proto::CostPerMilleReq &req, ad_proto::CostPerMilleResp &resp)
    {

        return 0;
    }
    int Ad::costPerAction(ad_proto::CostPerActionReq &req, ad_proto::CostPerActionResp &resp)
    {
        return 0;
    }
    int Ad::putAdvertise(ad_proto::PutAdvertiseReq &req, ad_proto::PutAdvertiseResp &resp)
    {
        return 0;
    }

    //----------------------------------------------------------
    int Ad::login(ad_proto::LoginReq &req, ad_proto::LoginResp &resp)
    {
        // 1. 处理入参
        string username = req.username();
        string password = req.password();
        char s[255];
        sprintf(s, "SELECT uuid FROM user WHERE username='%s' and password='%s';",
                username.c_str(), password.c_str());
        string sql(s);

        // 先查库有没有数据
        auto ret = MyDB::getInstance()->execSQL(sql);
        auto status = std::get<0>(ret);
        auto vec = std::get<1>(ret);

        if (status == errorEnum::SUCCESS)
        {
            if (vec.size() <= 0)
            {
                // 没有再注册
                // string uuid = common::genUUID();
                resp.set_err(errorEnum::NO_ACCOUNT);
                resp.set_msg("没有此账号");
                return errorEnum::NO_ACCOUNT;
            }
            else
            {
                auto tmp = vec[0];
                string uuid = tmp[0];
                resp.set_err(errorEnum::SUCCESS);
                resp.set_msg("登陆成功");
                resp.set_uuid(uuid);
                return errorEnum::SUCCESS;
            }
        }
        else
        {
            resp.set_err(errorEnum::MYSQL_QUERY_ERR);
            resp.set_msg("mysql查询失败");
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
        sprintf(s, "SELECT uuid FROM user WHERE username='%s' and password='%s';",
                username.c_str(), password.c_str());
        string sql(s);

        // 先查库有没有数据
        auto ret = MyDB::getInstance()->execSQL(sql);
        auto status = std::get<0>(ret);
        auto vec = std::get<1>(ret);

        string uuid;
        if (status == errorEnum::SUCCESS)
        {
            if (vec.size() <= 0)
            {
                // 没有再注册
                uuid = common::genUUID();
                resp.set_err(errorEnum::SUCCESS);
                resp.set_msg("注册成功");

                char buf[255];
                sprintf(buf, "INSERT INTO user VALUES(null,'%s','%s',%lu,'%s',%ld);",
                        username.c_str(), password.c_str(), phone, uuid.c_str(), time(NULL));

                sql = string(buf);
                MyDB::getInstance()->execSQL(sql);
            }
            else
            {
                auto tmp = vec[0];
                uuid = tmp[0];
                resp.set_err(errorEnum::HASBEEN_REGISTER);
                resp.set_msg("已注册");
            }
            resp.set_uuid(uuid);
        }
        else
        {
            resp.set_err(errorEnum::MYSQL_QUERY_ERR);
            resp.set_msg("mysql查询失败");
        }

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
        auto ret = MyDB::getInstance()->execSQL(sql);
        auto status = std::get<0>(ret);
        auto vec = std::get<1>(ret);

        if (status == errorEnum::SUCCESS)
        {
            if (vec.size() <= 0)
            {
                // 初始化资金
                char buf[255];
                sprintf(buf, "INSERT INTO money(id,uuid,amount,welfare) VALUES(null,'%s',%lf,%lf);",
                        uuid.c_str(), 0.0, 0.0);

                sql = string(buf);
                MyDB::getInstance()->execSQL(sql);

                resp.set_err(errorEnum::SUCCESS);
                resp.set_msg("查找成功");
                resp.set_uuid(uuid);
                resp.set_amount(0);
                resp.set_welfare(0);
            }
            else
            {
                auto tmp = vec[0];
                string uuid = tmp[1];
                double amount = strtod(tmp[2].c_str(), nullptr);
                double welfare = strtod(tmp[3].c_str(), nullptr);

                resp.set_err(errorEnum::SUCCESS);
                resp.set_msg("查找成功");
                resp.set_uuid(uuid);
                resp.set_amount(amount);
                resp.set_welfare(welfare);
                return errorEnum::SUCCESS;
            }
        }
        else
        {
            resp.set_err(errorEnum::MYSQL_QUERY_ERR);
            resp.set_msg("mysql查询失败");
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
        auto ret = MyDB::getInstance()->execSQL(sql);
        auto status = std::get<0>(ret);
        auto vec = std::get<1>(ret);

        if (status == errorEnum::SUCCESS)
        {
            if (vec.size() <= 0)
            {
                resp.set_err(errorEnum::MYSQL_QUERY_ERR);
                resp.set_msg("查找失败");
                return errorEnum::MYSQL_QUERY_ERR;
            }
            else
            {
                auto tmp = vec[0];
                string uuid = tmp[1];
                double amount = strtod(tmp[2].c_str(), nullptr);
                double welfare = strtod(tmp[3].c_str(), nullptr);
                amount += money;

                char buf[255];
                sprintf(buf, "UPDATE money SET amount=%lf WHERE uuid='%s';",
                        amount, uuid.c_str());

                sql = string(buf);
                MyDB::getInstance()->execSQL(sql);

                resp.set_err(errorEnum::SUCCESS);
                resp.set_msg("充值成功");
                resp.set_uuid(uuid);
                resp.set_amount(amount);
                resp.set_welfare(welfare);
                return errorEnum::SUCCESS;
            }
        }
        else
        {
            resp.set_err(errorEnum::MYSQL_QUERY_ERR);
            resp.set_msg("mysql查询失败");
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
        auto ret = MyDB::getInstance()->execSQL(sql);
        auto status = std::get<0>(ret);
        auto vec = std::get<1>(ret);

        if (status == errorEnum::SUCCESS)
        {
            if (vec.size() <= 0)
            {
                resp.set_err(errorEnum::MYSQL_QUERY_ERR);
                resp.set_msg("查找失败");
                return errorEnum::MYSQL_QUERY_ERR;
            }
            else
            {
                auto tmp = vec[0];
                string uuid = tmp[1];
                double amount = strtod(tmp[2].c_str(), nullptr);
                double welfare = strtod(tmp[3].c_str(), nullptr);
                amount -= money;

                char buf[255];
                sprintf(buf, "UPDATE money SET amount=%lf WHERE uuid='%s';",
                        amount, uuid.c_str());

                sql = string(buf);
                MyDB::getInstance()->execSQL(sql);

                resp.set_err(errorEnum::SUCCESS);
                resp.set_msg("扣费成功");
                resp.set_uuid(uuid);
                resp.set_amount(amount);
                resp.set_welfare(welfare);
                return errorEnum::SUCCESS;
            }
        }
        else
        {
            resp.set_err(errorEnum::MYSQL_QUERY_ERR);
            resp.set_msg("mysql查询失败");
        }

        return 0;
    }
}