#include "module.h"
#include <iostream>

namespace test{
        QueueServiceImpl::QueueServiceImpl(){};
        QueueServiceImpl::~QueueServiceImpl(){};
        void QueueServiceImpl::start(google::protobuf::RpcController *cntl_base,
                   const HttpRequest *,
                   HttpResponse *,
                   google::protobuf::Closure *done)
        {
            brpc::ClosureGuard done_guard(done);
            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);
            cntl->response_attachment().append("queue started");
            LOG(INFO) << "QueueService start";
        }
        void QueueServiceImpl::stop(google::protobuf::RpcController *cntl_base,
                  const HttpRequest *,
                  HttpResponse *,
                  google::protobuf::Closure *done)
        {
            brpc::ClosureGuard done_guard(done);
            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);
            cntl->response_attachment().append("queue stopped");
            LOG(INFO) << "QueueService start";
        }
        void QueueServiceImpl::getstats(google::protobuf::RpcController *cntl_base,
                      const HttpRequest *,
                      HttpResponse *,
                      google::protobuf::Closure *done)
        {
            LOG(INFO) << "QueueService getstats";
            brpc::ClosureGuard done_guard(done);
            brpc::Controller *cntl =
                static_cast<brpc::Controller *>(cntl_base);
            const std::string &unresolved_path = cntl->http_request().unresolved_path();
            if (unresolved_path.empty())
            {
                cntl->response_attachment().append("Require a name after /stats");
            }
            else
            {
                cntl->response_attachment().append("Get stats: ");
                cntl->response_attachment().append(unresolved_path);
            }
        }
}