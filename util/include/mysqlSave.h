#ifndef __MYSQL_SAVE_H__
#define __MYSQL_SAVE_H__
#include <iostream>
#include <gflags/gflags.h>
#include <brpc/channel.h>
#include <butil/logging.h>
#include "mysql.pb.h"
#include <tuple>
#include <vector>
#include <string>

DEFINE_string(attachment, "", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "127.0.0.1:9000", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");

void invoke(mysql_proto::SaveReq &request, mysql_proto::SaveResp &response)
{
    brpc::Channel channel;
    brpc::Controller cntl;
    brpc::ChannelOptions options;
    options.protocol = FLAGS_protocol;
    options.connection_type = FLAGS_connection_type;
    options.timeout_ms = FLAGS_timeout_ms /*milliseconds*/;
    options.max_retry = FLAGS_max_retry;
    if (channel.Init(FLAGS_server.c_str(), FLAGS_load_balancer.c_str(), &options) != 0)
    {
        LOG(ERROR) << "Fail to initialize channel";
    }
    mysql_proto::HttpService_Stub stub(&channel);
    stub.SaveDBV2(&cntl, &request, &response, NULL);
}
#endif