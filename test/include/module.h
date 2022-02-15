#ifndef __MUDOLE_H__
#define __MODULE_H__

#include <butil/logging.h>
#include <brpc/server.h>
#include "test.pb.h"

namespace test
{
    // Restful service. (The service implementation is exactly same with regular
    // services, the difference is that you need to pass a `restful_mappings'
    // when adding the service into server).
    class QueueServiceImpl : public test::QueueService
    {
    public:
        QueueServiceImpl();
        virtual ~QueueServiceImpl();
        void start(google::protobuf::RpcController *cntl_base,
                   const HttpRequest *,
                   HttpResponse *,
                   google::protobuf::Closure *done);

        void stop(google::protobuf::RpcController *cntl_base,
                  const HttpRequest *,
                  HttpResponse *,
                  google::protobuf::Closure *done);

        void getstats(google::protobuf::RpcController *cntl_base,
                      const HttpRequest *,
                      HttpResponse *,
                      google::protobuf::Closure *done);
    };
}
#endif