#include "common.h"

// 生成uuid
std::string genUUID()
{
    uuid_t uu;
    uuid_generate(uu);

    char uuid_str[37];
    uuid_unparse_lower(uu, uuid_str);

    return std::string(uuid_str);
}

// json转pb
void json2pb(const std::string &json, google::protobuf::Message &message)
{
    jsonpb::json2pb(message, json.c_str(), json.size());
}
// pb转json
std::string pb2json(const google::protobuf::Message &message)
{

    std::string data;
    Json::Value a;

    a = jsonpb::pb2json(message);
    return a.toStyledString();
}