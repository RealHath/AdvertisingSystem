#include <iostream>
#include <fstream>
#include <memory>
#include <sys/stat.h>
#include <mysql/mysql.h>

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include "mysql.pb.h"
#include "errorEnum.pb.h"
#include "sql.h"

DEFINE_string(ip_port, "127.0.0.1:9000", "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
                                 "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(logoff_ms, 2000, "Maximum duration of server's LOGOFF state "
                              "(waiting for client to close connection before server stops)");

DEFINE_string(url, "127.0.0.1", "mysql url");
DEFINE_int32(port, 3306, "mysql port");
DEFINE_string(user, "root", "mysql user");
DEFINE_string(password, "mysqlroot", "mysql password");
DEFINE_string(database, "ADSystem", "mysql database");

namespace mysql_proto
{

    // Service with static path.
    class HttpServiceImpl : public HttpService
    {
    public:
        HttpServiceImpl(){};
        virtual ~HttpServiceImpl(){};
        void SaveDBV2(google::protobuf::RpcController *cntl_base,
                      const SaveReq *req,
                      SaveResp *resp,
                      google::protobuf::Closure *done)
        {
            // This object helps you to call done->Run() in RAII style. If you need
            // to process the request asynchronously, pass done_guard.release().
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);
            // Fill response.
            LOG(INFO) << "sql: " << req->cmd();
            bool flag = MyDB::getInstance()->exeSQL(req->cmd());
            if (flag)
            {
                resp->set_err(0);
            }
            else
            {
                resp->set_err(errorEnum::SUCCESS);
            }
        }
    };
} // namespace mysql

// 日志配置
void configLog()
{
    const char *moduleName = "mysql";
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
    if (!flag)
    {
        LOG(ERROR) << "connect failed!";
    }
}

int main(int argc, char *argv[])
{
    // 守护进程
    daemon(1, 0);

    // Parse gflags. We recommend you to use gflags as well.
    GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);

    configLog();
    mysqlConfig();

    // Generally you only need one Server.
    brpc::Server server;

    mysql_proto::HttpServiceImpl http_svc;
    // fileTest::QueueServiceImpl queue_svc;

    // Add services into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(&http_svc,
                          brpc::SERVER_DOESNT_OWN_SERVICE,
                          "/mysql/SaveDBV2 => SaveDBV2") != 0)
    {
        LOG(ERROR) << "Fail to add http_svc";
        return -1;
    }

    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    if (server.Start(FLAGS_ip_port.c_str(), &options) != 0)
    {
        LOG(ERROR) << "Fail to start HttpServer";
        return -1;
    }
    LOG(INFO) << "mysql server start sucess";
    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    return 0;
}