#include <iostream>
#include <fstream>
#include <memory>
#include <sys/stat.h>
#include <mysql/mysql.h>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include "mysql.pb.h"
#include "errorEnum.pb.h"
#include "sql.h"

#include "common.h"

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
DEFINE_uint64(MAXSIZE, 1024, "queue maxsize");

// 线程同步
std::mutex g_mtx;
std::unique_lock<std::mutex> g_lock(g_mtx);
std::condition_variable g_cond;
std::queue<std::string> g_cmdQueue;

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
            if (g_cmdQueue.size() > FLAGS_MAXSIZE)
            {
                LOG(WARNING) << " sql queue curent size:" << g_cmdQueue.size();
            }

            string sql = req->cmd();
            LOG(INFO) << "sql: " << sql;
            auto res = MyDB::getInstance()->execSQL(sql);

            // 测试
            // g_cmdQueue.push(req->cmd());
            // auto res = MyDB::getInstance()->execSQL(g_cmdQueue.front());
            // g_cmdQueue.pop();

            auto vec = std::get<1>(res);

            if (vec.size())
            {
                for (size_t i = 0; i < vec.size(); ++i)
                {
                    if (vec[i].size())
                    {
                        auto *tmp = resp->add_info();
                        for (size_t j = 0; j < vec[i].size(); ++j)
                        {
                            if (vec[i][j].size())
                            {
                                tmp->add_field(vec[i][j]);
                            }
                        }
                    }
                }
            }
            resp->set_err(std::get<0>(res));
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
    if (flag)
    {
        LOG(INFO) << "connect mysql successed!";
    }
    else
    {
        LOG(ERROR) << "connect mysql failed!";
    }
}

static void mysqlThread()
{
    while (true)
    {
        // 阻塞等待
        g_cond.wait(g_lock, [&]()
                    { return g_cmdQueue.size(); });

        auto res = MyDB::getInstance()->execSQL(g_cmdQueue.front());
        g_cmdQueue.pop();

        if (std::get<0>(res) == errorEnum::SUCCESS)
        {
        }
        else
        {
        }
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
    // std::thread t(mysqlThread);
    // t.detach();

    // Generally you only need one Server.
    brpc::Server server;

    mysql_proto::HttpServiceImpl http_svc;
    // fileTest::QueueServiceImpl queue_svc;

    // Add services into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    // if (server.AddService(&http_svc,
    //                       brpc::SERVER_DOESNT_OWN_SERVICE,
    //                       "/mysql/SaveDBV2 => SaveDBV2") != 0)
    if (server.AddService(&http_svc,
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0)
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