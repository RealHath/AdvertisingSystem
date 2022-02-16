#include "module.h"
#include <iostream>
#include <json2pb/pb_to_json.h>
#include <json2pb/json_to_pb.h>

namespace test{
        QueueServiceImpl::QueueServiceImpl(){};
        QueueServiceImpl::~QueueServiceImpl(){};
        void QueueServiceImpl::start(const TestRequest& req, TestResponse& resp)
        {
            LOG(INFO) << "逻辑填充";
            resp.set_err(0);
            resp.set_message("测试" + req.name());
        }
        void QueueServiceImpl::stop(const TestRequest& req, TestResponse& resp)
        {
            LOG(INFO) << "QueueService start";
        }
        void QueueServiceImpl::getstats(const TestRequest& req, TestResponse& resp)
        {
            LOG(INFO) << "QueueService getstats";
        }
}