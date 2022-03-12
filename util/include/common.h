#ifndef __COMMON_H__
#define __COMMON_H__
#include <iostream>
// #include <google/protobuf/util/json_util.h>
// #include <google/protobuf/message.h>
#include <uuid/uuid.h>
#include "json2pb.h"

namespace common
{
    // json转pb
    void json2pb(const std::string &json, google::protobuf::Message &message);
    // pb转json
    std::string pb2json(const google::protobuf::Message &message);
    // 生成uuid
    std::string genUUID();

    std::string gb2312_to_utf8(std::string const &strGb2312);
    std::string utf8_to_gb2312(std::string const &strUtf8);

} // namespace common

#endif