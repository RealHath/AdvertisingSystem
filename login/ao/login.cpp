#include <butil/logging.h>
#include <gflags/gflags.h>
#include "login.h"
// #include "mysqlSave.h"
#include "common.h"
#include "mysql.pb.h"
#include <brpc/channel.h>

DEFINE_string(attachment, "", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "127.0.0.1:9000", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");

using namespace std;

namespace login_namespace
{
    Login::Login()
    {
    }
    Login::~Login()
    {
    }
    int Login::login(login_proto::LoginReq &req, login_proto::LoginResp &resp)
    {
        LOG(INFO) << "调用 login";
    }
    int Login::regist(login_proto::RegisterReq &req, login_proto::RegisterResp &resp)
    {
        // 1. 处理入参
        string username = req.username();
        string password = req.password();
        uint64_t phone = req.phone();

        string sql = "SELECT uuid FROM user WHERE username=" + username + " and password=" + password;

        // 先查库有没有数据
        mysql_proto::SaveReq request;
        mysql_proto::SaveResp response;
        request.set_cmd(sql);

        invoke(request, response);

        // 没有再注册
        string uuid = common::genUUID();

        resp.set_err(0);
        resp.set_msg("调用成功");
        resp.set_uuid(uuid);
    }

    void Login::invoke(mysql_proto::SaveReq &request, mysql_proto::SaveResp &response)
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