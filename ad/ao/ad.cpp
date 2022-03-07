#include <butil/logging.h>
#include <gflags/gflags.h>
#include <brpc/channel.h>
#include <time.h>
#include <unordered_map>
#include <memory>

#include "sql.h"
#include "errorEnum.pb.h"
#include "ad.h"
// #include "mysqlSave.h"
#include "common.h"
#include "mysql.pb.h"
#include "user.h"
#include "const.h"

using namespace std;
using namespace ad_namespace;

DEFINE_string(attachment, "", "Carry this along with requests");
DEFINE_string(protocol, "baidu_std", "Protocol type. Defined in src/brpc/options.proto");
DEFINE_string(connection_type, "", "Connection type. Available values: single, pooled, short");
DEFINE_string(server, "127.0.0.1:9000", "IP Address of server");
DEFINE_string(load_balancer, "", "The algorithm for load balancing");
DEFINE_int32(timeout_ms, 100, "RPC timeout in milliseconds");
DEFINE_int32(max_retry, 3, "Max retries(not including the first RPC)");
DEFINE_int32(interval_ms, 1000, "Milliseconds between consecutive requests");

// typedef std::shared_ptr<ad_namespace::ADUser> user_ptr;
// typedef std::shared_ptr<ad_namespace::Advertise> ad_ptr;
// typedef std::unordered_map<uint64_t, ad_ptr> ad_list;
// static std::unordered_map<std::string, user_ptr> g_userMap; // 用户映射表
// static std::unordered_map<std::string, ad_list> g_AUMap;    // 用户广告映射表

namespace ad_namespace
{
    Ad::Ad()
    {
    }
    Ad::~Ad()
    {
    }
    int Ad::costPerClick(ad_proto::CostPerClickReq &req, ad_proto::CostPerClickResp &resp)
    {
        // 1. 处理入参
        uint32_t id = req.id();
        auto ad = getAdvertise(id);
        if (ad == nullptr)
        {
            resp.set_err(errorEnum::NO_AD);
            resp.set_msg("没有此广告");
            return 0;
        }

        auto user = getUser(ad->uuid);
        if (user == nullptr)
        {
            initUser(ad->uuid);
            user = getUser(ad->uuid);
        }

        cout << "user->amount: " << user->amount;
        user->amount -= CLICK_COST;
        user->updateMoney(CLICK_COST * -1);
        ad->updateCost();

        resp.set_err(errorEnum::SUCCESS);
        resp.set_msg("CPC");
        return 0;
    }
    int Ad::costPerMille(ad_proto::CostPerMilleReq &req, ad_proto::CostPerMilleResp &resp)
    {

        uint32_t id = req.id();
        auto ad = getAdvertise(id);
        if (ad == nullptr)
        {
            resp.set_err(errorEnum::NO_AD);
            resp.set_msg("没有此广告");
            return 0;
        }

        auto user = getUser(ad->uuid);
        if (user == nullptr)
        {
            initUser(ad->uuid);
            user = getUser(ad->uuid);
        }

        user->amount -= MILLE_COST;
        user->updateMoney(MILLE_COST * -1);
        ad->updateCost();

        resp.set_err(errorEnum::SUCCESS);
        resp.set_msg("CPM");
        return 0;
    }

    int Ad::putAdvertise(ad_proto::PutAdvertiseReq &req, ad_proto::PutAdvertiseResp &resp)
    {
        // 1. 处理入参
        string uuid = req.uuid();
        string imageUrl = req.imageurl();
        string url = req.url();
        string content = req.content();
        uint32_t type = req.type();

        initUser(uuid);
        auto user = getUser(uuid);
        if (user == nullptr)
        {
            resp.set_err(errorEnum::NO_ACCOUNT);
            resp.set_msg("没有此账号");
            return 0;
        }

        uint32_t id = generateAdId();
        Advertise ad(id, uuid, imageUrl, url, content, type);
        ad_ptr ptr = make_shared<Advertise>(ad);
        ptr->insertAd();

        auto list = g_AUMap[uuid];
        list[id] = ptr;
        g_typeMap[type].push_back(ptr);
        g_adMap[id] = ptr;

        ::ad_proto::AdInfo *info = resp.mutable_info();
        info->set_id(id);
        info->set_imageurl(imageUrl);
        info->set_url(url);
        info->set_content(content);
        info->set_type(type);

        resp.set_err(errorEnum::SUCCESS);
        resp.set_msg("广告投放成功");

        return 0;
    }

    //随机返回一个广告
    int Ad::getAdInfo(ad_proto::GetAdInfoReq &req, ad_proto::GetAdInfoResp &resp)
    {
        // 1. 处理入参
        uint32_t type = req.type();

        if (g_AUMap.empty() || g_typeMap.empty())
        {
            g_AUMap.clear();
            g_typeMap.clear();

            char buf[2048];
            sprintf(buf, "SELECT * FROM ad WHERE type=%u;", type);
            auto ret = MyDB::getInstance()->execSQL(string(buf));
            for (size_t i = 0; i < ret.size(); i++)
            {
                auto m = ret[0];
                uint32_t id = strtoul(m["id"].c_str(), nullptr, 0);
                string uuid = m["uuid"];
                string url = m["url"];
                string imageUrl = m["imageUrl"];
                string content = m["content"];

                Advertise ad(id, uuid, imageUrl, url, content, type);
                auto ptr = make_shared<Advertise>(ad);
                g_AUMap[uuid].insert(make_pair(id, ptr));
                g_typeMap[type].push_back(ptr);
                g_adMap[id] = ptr;
            }
            LOG(INFO) << "g_typeMap.size(): " << g_typeMap.size();
        }
        if (g_typeMap[type].size() <= 0)
        {
            resp.set_err(errorEnum::NO_AD);
            resp.set_msg("没有广告");
            return 0;
        }

        // 随机取出一个广告
        size_t randVal = rand() % g_typeMap[type].size();
        auto ad = g_typeMap[type].at(randVal);

        uint32_t id = ad->id;
        string uuid = ad->uuid;
        string imageUrl = ad->imageUrl;
        string url = ad->url;
        string content = ad->content;
        // uint32_t type = ad->type;

        ::ad_proto::AdInfo *info = resp.mutable_info();
        info->set_id(id);
        info->set_imageurl(imageUrl);
        info->set_url(url);
        info->set_content(content);
        info->set_type(type);

        resp.set_err(errorEnum::SUCCESS);
        resp.set_msg("获取广告成功");

        return 0;
    }

    //获得用户
    user_ptr Ad::getUser(string uuid)
    {
        if (g_userMap.find(uuid) != g_userMap.end())
        {
            LOG(INFO) << "getUser: " << uuid << "g_userMap.size()" << g_userMap.size();
            return g_userMap[uuid];
        }
        return nullptr;
    }

    // 用户uuid，广告id
    std::shared_ptr<ad_namespace::Advertise> Ad::getAdvertise(uint32_t id)
    {
        if (g_adMap.find(id) != g_adMap.end())
        {
            return g_adMap[id];
        }

        char buf[2048];
        sprintf(buf, "SELECT * FROM ad WHERE id=%u;", id);
        auto ret = MyDB::getInstance()->execSQL(string(buf));
        if (ret.size())
        {
            auto m = ret[0];
            uint32_t id = strtoul(m["id"].c_str(), nullptr, 0);
            string uuid = m["uuid"];
            string url = m["url"];
            string imageUrl = m["imageUrl"];
            string content = m["content"];
            uint32_t type = strtoul(m["type"].c_str(), nullptr, 0);

            Advertise ad(id, uuid, imageUrl, url, content, type);
            auto ptr = make_shared<Advertise>(ad);
            g_adMap[id] = ptr;
            return g_adMap[id];
        }

        return nullptr;
    }

    // 将用户数据加载到内存
    bool Ad::initUser(string uuid)
    {
        if (g_AUMap.find(uuid) != g_AUMap.end() &&
            g_userMap.find(uuid) != g_userMap.end())
        {
            return true;
        }
        // 基本数据
        string username;
        string password;
        uint64_t phone;
        double amount = 0;
        double welfare = 0;

        char buf[2048];
        // 加载用户数据
        memset(buf, 0, 2048);
        sprintf(buf, "SELECT * FROM user WHERE uuid='%s';", uuid.c_str());
        string sql(buf);
        auto ret1 = MyDB::getInstance()->execSQL(sql);
        if (ret1.size())
        {
            username = ret1[0]["username"];
            password = ret1[0]["password"];
            phone = strtoul(ret1[0]["phone"].c_str(), nullptr, 0);
        }
        else
        {
            return false;
        }
        //资金数据
        memset(buf, 0, 2048);
        sprintf(buf, "SELECT * FROM money WHERE uuid='%s';", uuid.c_str());
        sql = string(buf);
        ret1 = MyDB::getInstance()->execSQL(sql);
        if (ret1.size())
        {
            amount = strtod(ret1[0]["amount"].c_str(), nullptr);
            welfare = strtod(ret1[0]["welfare"].c_str(), nullptr);
        }
        else
        {
            return false;
        }
        //广告数据
        ad_list list;

        ADUser user(uuid, username, password, phone, amount, welfare);
        user_ptr ptr = make_shared<ADUser>(user);
        g_userMap[uuid] = ptr;
        g_AUMap[uuid] = list;
        // g_adMap[id] = ptr;

        LOG(INFO) << "数据："
                  << " uuid:" << uuid << " username:" << username
                  << " password:" << password << " phone:" << phone
                  << " amount:" << amount << " welfare:" << welfare;
        LOG(INFO) << "g_userMap.size(): " << g_userMap.size();
        return true;
    }

    // 生成广告id
    uint32_t Ad::generateAdId()
    {
        char buf[2048];
        sprintf(buf, "SELECT COUNT(id) FROM ad WHERE 1=1;");
        auto ret = MyDB::getInstance()->execSQL(string(buf));

        uint32_t id = strtoul(ret[0]["count(id)"].c_str(), nullptr, 0);
        LOG(INFO) << "COUNT(id): " << id;
        return id + 1;
    }
}