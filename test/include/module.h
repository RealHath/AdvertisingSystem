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
        void start(const TestRequest& req, TestResponse& resp);

        void stop(const TestRequest& req, TestResponse& resp);

        void getstats(const TestRequest& req, TestResponse& resp);

    };
}
#endif