#ifndef __COMMON_H__
#define __COMMON_H__
#include <iostream>
// #include <google/protobuf/util/json_util.h>
// #include <google/protobuf/message.h>
#include <uuid/uuid.h>
#include "json2pb.h"

// json转pb
void json2pb(const std::string &json, google::protobuf::Message &message);
// pb转json
std::string pb2json(const google::protobuf::Message &message);
// 生成uuid
std::string genUUID();

#endif