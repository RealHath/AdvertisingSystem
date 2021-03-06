#include <iostream>
#include <fstream>
#include <memory>
extern "C"
{
#include <sys/stat.h>
}

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include <brpc/restful.h>
#include <json2pb/json_to_pb.h>
#include <json2pb/pb_to_json.h>
#include <brpc/http2.h>
#include "fileTest.pb.h"
#include "MultipartParser.h"

DEFINE_string(ip_port, "0.0.0.0:8020", "TCP Port of this server");
DEFINE_int32(idle_timeout_s, -1, "Connection will be closed if there is no "
                                 "read/write operations during the last `idle_timeout_s'");
DEFINE_int32(logoff_ms, 2000, "Maximum duration of server's LOGOFF state "
                              "(waiting for client to close connection before server stops)");

DEFINE_string(certificate, "../cert.pem", "Certificate file path to enable SSL");
DEFINE_string(private_key, "../key.pem", "Private key file path to enable SSL");
DEFINE_string(ciphers, "", "Cipher suite used for SSL connections");

namespace fileTest
{

    // Service with static path.
    class HttpServiceImpl : public HttpService
    {
    public:
        HttpServiceImpl(){};
        virtual ~HttpServiceImpl(){};
        void ImageUpload(google::protobuf::RpcController *cntl_base,
                         const HttpRequest *,
                         HttpResponse *,
                         google::protobuf::Closure *done)
        {
            // This object helps you to call done->Run() in RAII style. If you need
            // to process the request asynchronously, pass done_guard.release().
            brpc::ClosureGuard done_guard(done);

            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);
            // Fill response.

            FileUploadRequest req;
            FileUploadResponse resp;
            MultipartParser parser;

            LOG(INFO) << "query: " << cntl->http_request().uri().query();
            LOG(INFO) << "method: " << cntl->http_request().method();
            LOG(INFO) << "host: " << cntl->http_request().uri().host();
            LOG(INFO) << "path: " << cntl->http_request().uri().path();

            // ??????????????????
            std::string body = cntl->request_attachment().to_string();
            // std::string tmp;
            // getline(body, tmp);
            parser.setBoundary(body);
            size_t bufsize = body.size();
            char *buf = (char *)malloc(bufsize);

            // std::unique_ptr<char>
            //     buf = std::make_unique(body.size());
            // ???????????????????????????
            while (!parser.stopped())
            {
                // size_t len = fread(buf, 1, bufsize, f);
                size_t len = bufsize;
                size_t fed = 0;
                do
                {
                    size_t ret = parser.feed(buf + fed, len - fed);
                    fed += ret;
                } while (fed < len && !parser.stopped());
                // printf("???????????????");
            }
            // ?????????????????????
            parser.genReflectData();

            for (auto iter : parser.bodyData)
            {
                printf("body??????: %s\n", iter.c_str());
                LOG(INFO) << "body??????: " << iter.c_str();
            }
            for (auto iter : parser.reflectData)
            {
                printf("k v??????: %s, %s\n", iter.first.c_str(), iter.second.c_str());
                LOG(INFO) << "k v?????? : " << iter.first.c_str() << iter.second.c_str();
            }
            printf("%s\n", parser.getErrorMessage());

            LOG(INFO) << "post body: " << cntl->request_attachment();
            std::ofstream output;
            output.open("test.png", std::ios::out | std::ios::binary);
            output << cntl->request_attachment().to_string();

            std::string err;
            // json2pb::JsonToProtoMessage(body, (google::protobuf::Message *)&req, &err);
            if (!err.empty())
            {
                LOG(ERROR) << "err" << err;
            }

            // ????????????
            // cntl->http_response().set_content_type("application/json");
            cntl->http_response().SetHeader("Access-Control-Allow-Origin", "*");
            butil::IOBufBuilder os;
            os << "????????????";
            os.move_to(cntl->response_attachment());
        }
    };
} // namespace test

// ????????????
void configLog()
{
    const char *moduleName = "fileTest";
    // ??????
    mkdir("log", 0755);
    // time
    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    // log config
    char log_full_name[255];
    snprintf(log_full_name, 255, "log/%s_%d_%02d_%02d.log", moduleName, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
    std::string log_path = log_full_name;
    ::logging::LoggingSettings log_setting;          //??????LoggingSetting??????????????????
    log_setting.log_file = log_path.c_str();         //??????????????????
    log_setting.logging_dest = logging::LOG_TO_FILE; //????????????????????????????????????????????????
    ::logging::InitLogging(log_setting);             //??????????????????
}

int main(int argc, char *argv[])
{
    // ????????????
    // daemon(1, 0);

    // Parse gflags. We recommend you to use gflags as well.
    GFLAGS_NS::ParseCommandLineFlags(&argc, &argv, true);

    configLog();

    // Generally you only need one Server.
    brpc::Server server;

    // Instance of your service.
    fileTest::HttpServiceImpl http_svc;
    // fileTest::QueueServiceImpl queue_svc;

    // Add services into server. Notice the second parameter, because the
    // service is put on stack, we don't want server to delete it, otherwise
    // use brpc::SERVER_OWNS_SERVICE.
    if (server.AddService(&http_svc,
                          brpc::SERVER_DOESNT_OWN_SERVICE,
                          "/testFile/ImageUpload => ImageUpload") != 0)
    {
        LOG(ERROR) << "Fail to add http_svc";
        return -1;
    }
    // if (server.AddService(&queue_svc,
    //                       brpc::SERVER_DOESNT_OWN_SERVICE,
    //                       "/v1/queue/start   => start,"
    //                       "/v1/queue/stop    => stop,"
    //                       "/v1/queue/stats/* => getstats") != 0)
    // {
    //     LOG(ERROR) << "Fail to add queue_svc";
    //     return -1;
    // }

    // Start the server.
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_s;
    // https??????
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