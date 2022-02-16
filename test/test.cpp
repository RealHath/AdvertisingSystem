#include <iostream>
extern "C"
{
#include <sys/stat.h>
}

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include <brpc/restful.h>
#include "test.pb.h"
#include "module.h"

DEFINE_int32(port, 8010, "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
                                 "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(logoff_ms, 2000, "Maximum duration of server's LOGOFF state "
                              "(waiting for client to close connection before server stops)");

DEFINE_string(certificate, "cert.pem", "Certificate file path to enable SSL");
DEFINE_string(private_key, "key.pem", "Private key file path to enable SSL");
DEFINE_string(ciphers, "", "Cipher suite used for SSL connections");

namespace test
{

    // Service with static path.
    class HttpServiceImpl : public HttpService
    {
    public:
        HttpServiceImpl(){};
        virtual ~HttpServiceImpl(){};
        void Echo(google::protobuf::RpcController *cntl_base,
                  const HttpRequest *req,
                  HttpResponse *resp,
                  google::protobuf::Closure *done)
        {
            // This object helps you to call done->Run() in RAII style. If you need
            // to process the request asynchronously, pass done_guard.release().
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);
            // Fill response.
            if(req->has_name()){
                LOG(INFO) << "name, num:" << req->name() << req->num();
            }
            // cntl->http_response().set_content_type("text/plain");
            // butil::IOBufBuilder os;
            // os << "queries:";
            // for (brpc::URI::QueryIterator it = cntl->http_request().uri().QueryBegin();
            //      it != cntl->http_request().uri().QueryEnd(); ++it)
            // {
            //     os << ' ' << it->first << '=' << it->second;
            // }
            // os << "\nbody: " << cntl->request_attachment() << '\n';
            // LOG(INFO) << "HttpServiceImpl Echo" << os.buf();
            // os.move_to(cntl->response_attachment());

            resp->set_err(0);
            resp->set_message("protobuf test sucess");
        }
    };

    // Service with dynamic path.
    class FileServiceImpl : public FileService
    {
    public:
        FileServiceImpl(){};
        virtual ~FileServiceImpl(){};

        struct Args
        {
            butil::intrusive_ptr<brpc::ProgressiveAttachment> pa;
        };

        static void *SendLargeFile(void *raw_args)
        {
            std::unique_ptr<Args> args(static_cast<Args *>(raw_args));
            if (args->pa == NULL)
            {
                LOG(ERROR) << "ProgressiveAttachment is NULL";
                return NULL;
            }
            for (int i = 0; i < 100; ++i)
            {
                char buf[16];
                int len = snprintf(buf, sizeof(buf), "part_%d ", i);
                args->pa->Write(buf, len);

                // sleep a while to send another part.
                bthread_usleep(10000);
            }
            return NULL;
        }

        void default_method(google::protobuf::RpcController *cntl_base,
                            const HttpRequest *,
                            HttpResponse *,
                            google::protobuf::Closure *done)
        {
            brpc::ClosureGuard done_guard(done);
            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);
            const std::string &filename = cntl->http_request().unresolved_path();
            LOG(INFO) << "FileService default_method";
            if (filename == "largefile")
            {
                // Send the "largefile" with ProgressiveAttachment.
                std::unique_ptr<Args> args(new Args);
                args->pa = cntl->CreateProgressiveAttachment();
                bthread_t th;
                bthread_start_background(&th, NULL, SendLargeFile, args.release());
            }
            else
            {
                cntl->response_attachment().append("Getting file: ");
                cntl->response_attachment().append(filename);
            }
        }
    };
} // namespace test

// 日志配置
void configLog(){
    const char* moduleName = "test";
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

int main(int argc, char *argv[])
{
    // 守护进程
    daemon(1, 0);

    // Parse gflags. We recommend you to use gflags as well.
    GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);

    configLog();

    // Generally you only need one Server.
    brpc::Server server;

    // Instance of your service.
    test::HttpServiceImpl http_svc;
    test::FileServiceImpl file_svc;
    test::QueueServiceImpl queue_svc;

    // Add services into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(&http_svc,
                          brpc::SERVER_DOESNT_OWN_SERVICE,
                          "/v1/test => Echo") != 0)
    {
        LOG(ERROR) << "Fail to add http_svc";
        return -1;
    }
    if (server.AddService(&file_svc,
                          brpc::SERVER_DOESNT_OWN_SERVICE) != 0)
    {
        LOG(ERROR) << "Fail to add file_svc";
        return -1;
    }
    if (server.AddService(&queue_svc,
                          brpc::SERVER_DOESNT_OWN_SERVICE,
                          "/v1/queue/start   => start,"
                          "/v1/queue/stop    => stop,"
                          "/v1/queue/stats/* => getstats") != 0)
    {
        LOG(ERROR) << "Fail to add queue_svc";
        return -1;
    }

    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    // https相关
    // options.mutable_ssl_options()->default_cert.certificate = FLAGS_certificate;
    // options.mutable_ssl_options()->default_cert.private_key = FLAGS_private_key;
    // options.mutable_ssl_options()->ciphers = FLAGS_ciphers;
    if (server.Start(FLAGS_port, &options) != 0)
    {
        LOG(ERROR) << "Fail to start HttpServer";
        return -1;
    }
    LOG(INFO) << "http server start sucess";
    // Wait until Ctrl-C is pressed, then Stop() and Join() the server.
    server.RunUntilAskedToQuit();
    return 0;
}