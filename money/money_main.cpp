#include <iostream>
#include <fstream>
#include <memory>
#include <sys/stat.h>

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include <brpc/restful.h>
#include <json2pb/json_to_pb.h>
#include <json2pb/pb_to_json.h>
#include <brpc/http2.h>

#include "money.pb.h"
#include "money.h"
#include "common.h"

DEFINE_string(ip_port, "127.0.0.1:10002", "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
                                 "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(logoff_ms, 2000, "Maximum duration of server's LOGOFF state "
                              "(waiting for client to close connection before server stops)");

DEFINE_string(certificate, "../cert.pem", "Certificate file path to enable SSL");
DEFINE_string(private_key, "../key.pem", "Private key file path to enable SSL");
DEFINE_string(ciphers, "", "Cipher suite used for SSL connections");

money_namespace::Money *ao;

// 日志配置
void configLog()
{
    const char *moduleName = "money";
    // 日志
    mkdir("log", 0755);
    // time
    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    // log config
    char log_full_name[255];
    snprintf(log_full_name, 255, "log/%s_%d_%02d_%02d.log", moduleName, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
    std::string log_path = log_full_name;
    ::logging::LoggingSettings log_setting;          //创建LoggingSetting对象进行设置
    log_setting.log_file = log_path.c_str();         //设置日志路径
    log_setting.logging_dest = logging::LOG_TO_FILE; //设置日志写到文件，不写的话不生效
    ::logging::InitLogging(log_setting);             //应用日志设置
}

namespace money_proto
{
    class HttpServiceImpl : public HttpService
    {
    public:
        HttpServiceImpl(){};
        virtual ~HttpServiceImpl(){};
        void GetFundInfo(google::protobuf::RpcController *cntl_base,
                         const HttpRequest *,
                         HttpResponse *,
                         google::protobuf::Closure *done)
        {
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);

            money_proto::GetFundInfoReq req;
            money_proto::GetFundInfoResp resp;

            // 请求
            std::string body = cntl->request_attachment().to_string();
            common::json2pb(body, req);

            // 逻辑处理入口
            ao->getFundInfo(req, resp);

            // 返回
            std::string respData = common::pb2json(resp);

            // 返回前端
            cntl->http_response().set_content_type("application/json");
            butil::IOBufBuilder os;
            os << respData;
            os.move_to(cntl->response_attachment());
        }

        void Recharge(google::protobuf::RpcController *cntl_base,
                      const HttpRequest *,
                      HttpResponse *,
                      google::protobuf::Closure *done)
        {
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);

            money_proto::RechargeReq req;
            money_proto::RechargeResp resp;

            std::string body = cntl->request_attachment().to_string();
            common::json2pb(body, req);

            // 逻辑处理入口
            ao->recharge(req, resp);

            // 返回
            std::string respData = common::pb2json(resp);

            // 返回前端
            cntl->http_response()
                .set_content_type("application/json");
            butil::IOBufBuilder os;
            os << respData;
            os.move_to(cntl->response_attachment());
        }

        void Deduction(google::protobuf::RpcController *cntl_base,
                       const HttpRequest *,
                       HttpResponse *,
                       google::protobuf::Closure *done)
        {
        }
    };
}

int main(int argc, char *argv[])
{
    // 守护进程
    daemon(1, 0);

    // Parse gflags. We recommend you to use gflags as well.
    GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);

    configLog();
    ao = new money_namespace::Money;

    // Generally you only need one Server.
    brpc::Server server;

    // Instance of your service.

    money_proto::HttpServiceImpl http_svc;

    // Add services into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(&http_svc,
                          brpc::SERVER_DOESNT_OWN_SERVICE,
                          "/money/GetFundInfo => GetFundInfo,"
                          "/money/Recharge => Recharge,"
                          "/money/Deduction => Deduction") != 0)
    {
        LOG(ERROR) << "Fail to add http_svc";
        return -1;
    }

    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    // https相关
    options.mutable_ssl_options()->default_cert.certificate = FLAGS_certificate;
    options.mutable_ssl_options()->default_cert.private_key = FLAGS_private_key;
    options.mutable_ssl_options()->ciphers = FLAGS_ciphers;
    if (server.Start(FLAGS_ip_port.c_str(), &options) != 0)
    {
        LOG(ERROR) << "Fail to start HttpServer";
        return -1;
    }
    LOG(INFO) << "http server start sucess";
    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    return 0;
}