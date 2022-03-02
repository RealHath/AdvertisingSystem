#include <iostream>
#include <fstream>
#include <memory>
#include <sys/stat.h>
#include <chrono>
#include <thread>

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include <brpc/restful.h>
#include <brpc/http2.h>

#include "ad.h"
#include "common.h"
#include "sql.h"

using namespace std;

// ip
DEFINE_string(ip_port, "127.0.0.1:10003", "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
                                 "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(logoff_ms, 2000, "Maximum duration of server's LOGOFF state "
                              "(waiting for client to close connection before server stops)");

// https
DEFINE_string(certificate, "../cert.pem", "Certificate file path to enable SSL");
DEFINE_string(private_key, "../key.pem", "Private key file path to enable SSL");
DEFINE_string(ciphers, "", "Cipher suite used for SSL connections");

// mysql
DEFINE_string(url, "127.0.0.1", "mysql url");
DEFINE_int32(port, 3306, "mysql port");
DEFINE_string(user, "root", "mysql user");
DEFINE_string(password, "mysqlroot", "mysql password");
DEFINE_string(database, "ADSystem", "mysql database");
DEFINE_uint64(MAXSIZE, 1024, "queue maxsize");

ad_namespace::Ad *ad_ao;

namespace ad_proto
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

            ad_proto::GetFundInfoReq req;
            ad_proto::GetFundInfoResp resp;

            // 请求
            std::string body = cntl->request_attachment().to_string();
            cout << body << endl;
            common::json2pb(body, req);

            // 逻辑处理入口
            ad_ao->getFundInfo(req, resp);

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

            ad_proto::RechargeReq req;
            ad_proto::RechargeResp resp;

            std::string body = cntl->request_attachment().to_string();
            common::json2pb(body, req);

            // 逻辑处理入口
            ad_ao->recharge(req, resp);

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
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);

            ad_proto::DeductionReq req;
            ad_proto::DeductionResp resp;

            std::string body = cntl->request_attachment().to_string();
            common::json2pb(body, req);

            // 逻辑处理入口
            ad_ao->deduction(req, resp);

            // 返回
            std::string respData = common::pb2json(resp);

            // 返回前端
            cntl->http_response()
                .set_content_type("application/json");
            butil::IOBufBuilder os;
            os << respData;
            os.move_to(cntl->response_attachment());
        }

    public:
        void Register(google::protobuf::RpcController *cntl_base,
                      const HttpRequest *,
                      HttpResponse *,
                      google::protobuf::Closure *done)
        {
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);

            ad_proto::RegisterReq req;
            ad_proto::RegisterResp resp;

            // 请求
            std::string body = cntl->request_attachment().to_string();
            common::json2pb(body, req);

            // 逻辑处理入口
            ad_ao->regist(req, resp);

            // 返回
            std::string respData = common::pb2json(resp);

            // 返回前端
            cntl->http_response().set_content_type("application/json");
            butil::IOBufBuilder os;
            os << respData;
            os.move_to(cntl->response_attachment());
        }

        void Login(google::protobuf::RpcController *cntl_base,
                   const HttpRequest *,
                   HttpResponse *,
                   google::protobuf::Closure *done)
        {
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);

            ad_proto::LoginReq req;
            ad_proto::LoginResp resp;

            std::string body = cntl->request_attachment().to_string();
            common::json2pb(body, req);

            // 逻辑处理入口
            ad_ao->login(req, resp);

            // 返回
            std::string respData = common::pb2json(resp);

            // 返回前端
            cntl->http_response()
                .set_content_type("application/json");
            butil::IOBufBuilder os;
            os << respData;
            os.move_to(cntl->response_attachment());
        }

    public:
        void CostPerClick(google::protobuf::RpcController *cntl_base,
                          const HttpRequest *,
                          HttpResponse *,
                          google::protobuf::Closure *done)
        {
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);

            ad_proto::CostPerClickReq req;
            ad_proto::CostPerClickResp resp;

            // 请求
            std::string body = cntl->request_attachment().to_string();
            common::json2pb(body, req);

            // 逻辑处理入口
            ad_ao->costPerClick(req, resp);

            // 返回
            std::string respData = common::pb2json(resp);

            // 返回前端
            cntl->http_response().set_content_type("application/json");
            butil::IOBufBuilder os;
            os << respData;
            os.move_to(cntl->response_attachment());
        }

        void CostPerMille(google::protobuf::RpcController *cntl_base,
                          const HttpRequest *,
                          HttpResponse *,
                          google::protobuf::Closure *done)
        {
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);

            ad_proto::CostPerMilleReq req;
            ad_proto::CostPerMilleResp resp;

            std::string body = cntl->request_attachment().to_string();
            common::json2pb(body, req);

            // 逻辑处理入口
            ad_ao->costPerMille(req, resp);

            // 返回
            std::string respData = common::pb2json(resp);

            // 返回前端
            cntl->http_response()
                .set_content_type("application/json");
            butil::IOBufBuilder os;
            os << respData;
            os.move_to(cntl->response_attachment());
        }

        void CostPerAction(google::protobuf::RpcController *cntl_base,
                           const HttpRequest *,
                           HttpResponse *,
                           google::protobuf::Closure *done)
        {
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);

            ad_proto::CostPerActionReq req;
            ad_proto::CostPerActionResp resp;

            std::string body = cntl->request_attachment().to_string();
            common::json2pb(body, req);

            // 逻辑处理入口
            ad_ao->costPerAction(req, resp);

            // 返回
            std::string respData = common::pb2json(resp);

            // 返回前端
            cntl->http_response()
                .set_content_type("application/json");
            butil::IOBufBuilder os;
            os << respData;
            os.move_to(cntl->response_attachment());
        }

        void PutAdvertise(google::protobuf::RpcController *cntl_base,
                          const HttpRequest *,
                          HttpResponse *,
                          google::protobuf::Closure *done)
        {
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);

            ad_proto::PutAdvertiseReq req;
            ad_proto::PutAdvertiseResp resp;

            std::string body = cntl->request_attachment().to_string();
            common::json2pb(body, req);

            // 逻辑处理入口
            ad_ao->putAdvertise(req, resp);

            // 返回
            std::string respData = common::pb2json(resp);

            // 返回前端
            cntl->http_response()
                .set_content_type("application/json");
            butil::IOBufBuilder os;
            os << respData;
            os.move_to(cntl->response_attachment());
        }
    };
}

// 日志配置
void configLog()
{
    const char *moduleName = "ad";
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

void mysqlConfig()
{
    bool flag = MyDB::getInstance()->connect(FLAGS_url, FLAGS_port, FLAGS_user, FLAGS_password, FLAGS_database);
    if (flag)
    {
        LOG(INFO) << "connect mysql successed!";
    }
    else
    {
        LOG(ERROR) << "connect mysql failed!";
    }
}

// ping mysql
void pingMysql(void *)
{
    LOG(INFO) << "start pingMysql thread!";
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::hours(7));
        // std::this_thread::sleep_for(std::chrono::seconds(7));
        auto ret = MyDB::getInstance()->execSQL("PING");
        if (std::get<0>(ret) != errorEnum::SUCCESS)
        {
            LOG(ERROR) << "mysql ping err!!!!";
        }
    }
}

int main(int argc, char *argv[])
{
    // 守护进程
    // daemon(1, 0);

    // Parse gflags. We recommend you to use gflags as well.
    GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);

    // 个人配置
    configLog();
    ad_ao = new ad_namespace::Ad;
    mysqlConfig();
    std::thread t(&pingMysql, nullptr);
    t.detach();

    // Generally you only need one Server.
    brpc::Server server;

    // Instance of your service.

    ad_proto::HttpServiceImpl ad_svc;
    // ad_proto::HttpServiceImpl money_svc;
    // ad_proto::HttpServiceImpl login_svc;

    // Add services into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(&ad_svc,
                          brpc::SERVER_DOESNT_OWN_SERVICE,
                          "/ad/CostPerClick => CostPerClick,"
                          "/ad/CostPerMille => CostPerMille,"
                          "/ad/CostPerAction => CostPerAction,"
                          "/money/GetFundInfo => GetFundInfo,"
                          "/money/Recharge => Recharge,"
                          "/money/Deduction => Deduction,"
                          "/login/Register => Register,"
                          "/login/Login => Login,") != 0)
    {
        LOG(ERROR) << "Fail to add ad_svc";
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