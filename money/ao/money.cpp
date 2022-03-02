#include <butil/logging.h>
#include <gflags/gflags.h>
#include "money.h"
// #include "mysqlSave.h"
#include "common.h"
#include "mysql.pb.h"
#include <brpc/channel.h>
#include "errorEnum.pb.h"
#include <time.h>

DEFINE_string(attachment, "", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "127.0.0.1:9000", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");

using namespace std;

namespace money_namespace
{
    Money::Money()
    {
    }
    Money::~Money()
    {
    }
    int Money::getFundInfo(money_proto::GetFundInfoReq &req, money_proto::GetFundInfoResp &resp)
    {
        // 1. 处理入参
        string uuid = req.uuid();

        char s[255];
        sprintf(s, "SELECT * FROM money WHERE uuid='%s';",
                uuid.c_str());
        string sql(s);

        // 先查库有没有数据
        mysql_proto::SaveReq request;
        mysql_proto::SaveResp response;
        request.set_cmd(sql);

        invoke(request, response);

        if (response.err() == errorEnum::SUCCESS)
        {
            if (response.info_size() <= 0)
            {
                // 初始化资金
                char buf[255];
                sprintf(buf, "INSERT INTO money(id,uuid,amount,welfare) VALUES(null,'%s',%lf,%lf);",
                        uuid.c_str(), 0.0, 0.0);

                sql = string(buf);
                request.clear_cmd();
                request.set_cmd(sql);
                invoke(request, response);

                resp.set_err(errorEnum::SUCCESS);
                resp.set_msg("查找成功");
                resp.set_uuid(uuid);
                resp.set_amount(0);
                resp.set_welfare(0);
            }
            else
            {
                auto tmp = response.info(0);
                string uuid = tmp.field(1);
                double amount = strtod(tmp.field(2).c_str(), nullptr);
                double welfare = strtod(tmp.field(3).c_str(), nullptr);

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

        return response.err();
    }
    int Money::recharge(money_proto::RechargeReq &req, money_proto::RechargeResp &resp)
    {
        // 1. 处理入参
        string uuid = req.uuid();
        double money = req.money();

        char s[255];
        sprintf(s, "SELECT * FROM money WHERE uuid='%s';",
                uuid.c_str());
        string sql(s);

        // 先查库有没有数据
        mysql_proto::SaveReq request;
        mysql_proto::SaveResp response;
        request.set_cmd(sql);
        invoke(request, response);

        if (response.err() == errorEnum::SUCCESS)
        {
            if (response.info_size() <= 0)
            {
                resp.set_err(errorEnum::MYSQL_QUERY_ERR);
                resp.set_msg("查找失败");
                return errorEnum::MYSQL_QUERY_ERR;
            }
            else
            {
                auto tmp = response.info(0);
                string uuid = tmp.field(1);
                double amount = strtod(tmp.field(2).c_str(), nullptr);
                double welfare = strtod(tmp.field(3).c_str(), nullptr);
                amount += money;

                char buf[255];
                sprintf(buf, "UPDATE money SET amount=%lf WHERE uuid='%s';",
                        amount, uuid.c_str());

                sql = string(buf);
                request.clear_cmd();
                request.set_cmd(sql);
                invoke(request, response);

                resp.set_err(errorEnum::SUCCESS);
                resp.set_msg("充值成功");
                resp.set_uuid(uuid);
                resp.set_amount(amount);
                resp.set_welfare(welfare);
                return errorEnum::SUCCESS;
            }
            resp.set_uuid(uuid);
        }
        else
        {
            resp.set_err(errorEnum::MYSQL_QUERY_ERR);
            resp.set_msg("mysql查询失败");
        }

        return response.err();
    }
    int Money::deduction(money_proto::DeductionReq &req, money_proto::DeductionResp &resp)
    {
        // 1. 处理入参
        string uuid = req.uuid();
        double money = req.money();

        char s[255];
        sprintf(s, "SELECT * FROM money WHERE uuid='%s';",
                uuid.c_str());
        string sql(s);

        // 先查库有没有数据
        mysql_proto::SaveReq request;
        mysql_proto::SaveResp response;
        request.set_cmd(sql);
        invoke(request, response);

        if (response.err() == errorEnum::SUCCESS)
        {
            if (response.info_size() <= 0)
            {
                resp.set_err(errorEnum::MYSQL_QUERY_ERR);
                resp.set_msg("查找失败");
                return errorEnum::MYSQL_QUERY_ERR;
            }
            else
            {
                auto tmp = response.info(0);
                string uuid = tmp.field(1);
                double amount = strtod(tmp.field(2).c_str(), nullptr);
                double welfare = strtod(tmp.field(3).c_str(), nullptr);
                amount -= money;

                char buf[255];
                sprintf(buf, "UPDATE money SET amount=%lf WHERE uuid='%s';",
                        amount, uuid.c_str());

                sql = string(buf);
                request.clear_cmd();
                request.set_cmd(sql);
                invoke(request, response);

                resp.set_err(errorEnum::SUCCESS);
                resp.set_msg("扣费成功");
                resp.set_uuid(uuid);
                resp.set_amount(amount);
                resp.set_welfare(welfare);
                return errorEnum::SUCCESS;
            }
            resp.set_uuid(uuid);
        }
        else
        {
            resp.set_err(errorEnum::MYSQL_QUERY_ERR);
            resp.set_msg("mysql查询失败");
        }

        return response.err();
    }

    void Money::invoke(mysql_proto::SaveReq &request, mysql_proto::SaveResp &response)
    {
        brpc::Channel channel;
        brpc::Controller cntl;
        brpc::ChannelOptions options;
        options.protocol = FLAGS_protocol;
        options.connection_type = FLAGS_connection_type;
        options.timeout_ms = FLAGS_timeout_ms /*milliseconds*/;
        options.max_retry = FLAGS_max_retry;
        if (channel.Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0)
        {
            LOG(ERROR) << "Fail to initialize channel";
        }
        mysql_proto::HttpService_Stub stub(&channel);
        stub.SaveDBV2(&cntl, &request, &response, NULL);
    }
}