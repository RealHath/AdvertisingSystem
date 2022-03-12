#include "common.h"
#include <codecvt>
#include <locale>
namespace common
{
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

    std::string gb2312_to_utf8(std::string const &strGb2312)
    {
        std::vector<wchar_t> buff(strGb2312.size());
#ifdef _MSC_VER
        std::locale loc("zh-CN");
#else
        std::locale loc("zh_CN.GB18030");
#endif
        wchar_t *pwszNext = nullptr;
        const char *pszNext = nullptr;
        mbstate_t state = {};
        int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t>>(loc).in(state,
                                                                                 strGb2312.data(), strGb2312.data() + strGb2312.size(), pszNext,
                                                                                 buff.data(), buff.data() + buff.size(), pwszNext);
        if (std::codecvt_base::ok == res)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
            return cutf8.to_bytes(std::wstring(buff.data(), pwszNext));
        }
        return "";
    }
    std::string utf8_to_gb2312(std::string const &strUtf8)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
        std::wstring wTemp = cutf8.from_bytes(strUtf8);
#ifdef _MSC_VER
        std::locale loc("zh-CN");
#else
        std::locale loc("zh_CN.GB18030");
#endif
        const wchar_t *pwszNext = nullptr;
        char *pszNext = nullptr;
        mbstate_t state = {};
        std::vector<char> buff(wTemp.size() * 2);
        int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t>>(loc).out(state,
                                                                                  wTemp.data(), wTemp.data() + wTemp.size(), pwszNext,
                                                                                  buff.data(), buff.data() + buff.size(), pszNext);
        if (std::codecvt_base::ok == res)
        {
            return std::string(buff.data(), pszNext);
        }
        return "";
    }
}