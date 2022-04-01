#include <butil/logging.h>
#include <gflags/gflags.h>
#include <brpc/channel.h>
#include <time.h>
#include <unordered_map>
#include <memory>
#include <codecvt>
#include <set>
#include <algorithm>
#include <mutex>
#include <random>
#include <queue>
#include <cmath>

#include "sql.h"
#include "errorEnum.pb.h"
#include "ad.h"
// #include "mysqlSave.h"
#include "common.h"
// #include "mysql.pb.h"
#include "user.h"
#include "const.h"
#include "md5.h"

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

// extern std::unordered_map<uint32_t, vector<ad_ptr>> g_typeMap; // 类型广告映射表
// extern std::unordered_map<std::string, user_ptr> g_userMap; // 用户映射表
// extern std::unordered_map<uint32_t, ad_ptr> g_adMap;     // 用户广告映射表
// extern std::unordered_map<std::string, count_ptr> g_countMap; // 统计表
// extern std::unordered_map<std::string, ad_list> g_AUMap;       // 用户广告映射表

std::unordered_map<std::string, user_ptr> g_userMap;                         // 用户映射表
std::unordered_map<uint32_t, ad_ptr> g_adMap;                                // 用户广告映射表
std::unordered_map<std::string, ad_list> g_AUMap;                            // 用户广告映射表
std::unordered_map<uint32_t, vector<ad_ptr>> g_typeMap;                      // 类型广告映射表
std::unordered_map<std::string, count_ptr> g_countMap;                       // 统计表
std::unordered_map<uint32_t, std::unordered_map<uint32_t, int>> g_weightMap; //权重表

std::mutex init_mtx;
// default_random_engine e(time(NULL));

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
        if (!req.has_id())
        {
            resp.set_err(errorEnum::EMPTY_ADID);
            resp.set_msg("广告id空");
            return 0;
        }

        // 1. 处理入参
        uint32_t id = req.id();
        auto ad = getAdvertise(id);
        if (ad == nullptr)
        {
            resp.set_err(errorEnum::NO_AD);
            resp.set_msg("没有此广告");
            return 0;
        }

        if (ad->type != errorEnum::CLICK)
        {
            resp.set_err(errorEnum::TYPE_NE_ID);
            resp.set_msg("广告id类型不匹配接口");
            return 0;
        }

        if (ad->status == errorEnum::NOT_ADUIT)
        {
            resp.set_err(errorEnum::HAS_PUTDOWN);
            resp.set_msg("广告已下架");
            return 0;
        }

        auto user = getUser(ad->uuid);
        if (user == nullptr)
        {
            initUser(ad->uuid);
            user = getUser(ad->uuid);
        }
        // uniform_real_distribution<int> u(-5, 10);
        int a = rand() % 20 - 10;
        double cost = CLICK_COST + a / 100.0;

        g_countMap[ad->uuid]->countAdd(ad->type);
        user->amount -= cost;
        user->updateMoney(cost * -1);
        ad->updateCost(cost);

        if (user->amount < cost)
        {
            ad->status = errorEnum::NOT_ADUIT;
            user->changeAdStatus(errorEnum::NOT_ADUIT, ad->type);
            // g_typeMap[ad->type].clear();
        }

        resp.set_err(errorEnum::SUCCESS);
        resp.set_msg("CPC");
        return 0;
    }
    int Ad::costPerMille(ad_proto::CostPerMilleReq &req, ad_proto::CostPerMilleResp &resp)
    {
        if (!req.has_id())
        {
            resp.set_err(errorEnum::EMPTY_ADID);
            resp.set_msg("广告id空");
            return 0;
        }

        uint32_t id = req.id();
        auto ad = getAdvertise(id);
        if (ad == nullptr)
        {
            resp.set_err(errorEnum::NO_AD);
            resp.set_msg("没有此广告");
            return 0;
        }

        if (ad->type != errorEnum::MILLE)
        {
            resp.set_err(errorEnum::TYPE_NE_ID);
            resp.set_msg("广告id类型不匹配接口");
            return 0;
        }

        if (ad->status == errorEnum::NOT_ADUIT)
        {
            resp.set_err(errorEnum::HAS_PUTDOWN);
            resp.set_msg("广告已下架");
            return 0;
        }

        auto user = getUser(ad->uuid);
        if (user == nullptr)
        {
            initUser(ad->uuid);
            user = getUser(ad->uuid);
        }
        // uniform_real_distribution<int> u(-5, 10);
        int a = rand() % 10 - 5;
        double cost = (MILLE_COST + a) / 1000;

        g_countMap[ad->uuid]->countAdd(ad->type);
        user->amount -= cost;
        user->updateMoney(cost / 1000 * -1);
        ad->updateCost(cost);

        if (user->amount < cost)
        {
            ad->status = errorEnum::NOT_ADUIT;
            user->changeAdStatus(errorEnum::NOT_ADUIT, ad->type);
            // g_typeMap[ad->type].clear();
        }

        resp.set_err(errorEnum::SUCCESS);
        resp.set_msg("CPM");
        return 0;
    }

    int Ad::putAdvertise(ad_proto::PutAdvertiseReq &req, ad_proto::PutAdvertiseResp &resp)
    {
        if (!req.has_uuid() || !req.has_type() || !req.has_imageurl())
        {
            resp.set_err(errorEnum::INPUT_ERROR);
            resp.set_msg("输入错误");
            return 0;
        }

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

        // 资金够了才能过审
        uint32_t status = errorEnum::NOT_ADUIT;
        if (type == errorEnum::CLICK && user->amount > CLICK_COST)
        {
            status = errorEnum::ADUITED;
        }
        else if (type == errorEnum::MILLE && user->amount > MILLE_COST / 1000.0)
        {
            status = errorEnum::ADUITED;
        }
        else if (type == errorEnum::VISIT && user->amount > VISIT_COST)
        {
            status = errorEnum::ADUITED;
        }
        else if (type == errorEnum::SHOP && user->amount > SHOP_COST)
        {
            status = errorEnum::ADUITED;
        }
        else if (type == errorEnum::SELL && user->amount > SELL_COST)
        {
            status = errorEnum::ADUITED;
        }

        // get参数扰乱
        char param[1024];
        uint32_t id = generateAdId();
        string noice = MD5(string(common::genUUID())).md5();
        if (url.rfind('?') != string::npos)
        {
            sprintf(param, "&adId=%u&noice=%s", id, noice.c_str());
            url.append(string(param));
        }
        else
        {
            sprintf(param, "?id=%u&noice=%s", id, noice.c_str());
            url.append(string(param));
        }

        Advertise ad(id, uuid, imageUrl, url, content, type, status);
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
        if (!req.has_type() || !req.has_num())
        {
            resp.set_err(errorEnum::INPUT_ERROR);
            resp.set_msg("输入错误");
            return 0;
        }

        // 1. 处理入参
        uint32_t type = req.type();
        uint32_t num = req.num();
        // uint32_t type = 1;
        // uint32_t num = 4;

        if (num <= 0 || !checkType(type))
        {
            resp.set_err(errorEnum::INPUT_ERROR);
            resp.set_msg("输入错误");
            return 0;
        }

        if (g_typeMap[type].empty())
        // if (g_typeMap[type].empty())
        {
            // g_AUMap.clear();
            g_typeMap[type].clear();
            g_weightMap[type].clear();

            char buf[2048];
            sprintf(buf, "SELECT * FROM ad WHERE type=%u AND status=%u;", type, errorEnum::ADUITED);
            // sprintf(buf, "SELECT * FROM ad;");
            auto ret = MyDB::getInstance()->syncExecSQL(string(buf));
            for (size_t i = 0; i < ret.size(); i++)
            {
                auto m = ret[i];
                uint32_t id = strtoul(m["id"].c_str(), nullptr, 0);
                string uuid = m["uuid"];
                string url = m["url"];
                string imageUrl = m["imageUrl"];
                string content = m["content"];
                uint32_t status = strtoul(m["status"].c_str(), nullptr, 0);

                Advertise ad(id, uuid, imageUrl, url, content, type, status);
                auto ptr = make_shared<Advertise>(ad);
                g_AUMap[uuid].insert(make_pair(id, ptr));
                g_typeMap[type].push_back(ptr);
                g_adMap[id] = ptr;

                // weight
                g_weightMap[type].insert(make_pair(id, 8192));
            }
            LOG(INFO) << "g_typeMap[type].size(): " << g_typeMap[type].size();
            LOG(INFO) << "g_weightMap[type].size(): " << g_weightMap[type].size();
        }
        if (g_typeMap[type].size() <= 0)
        {
            resp.set_err(errorEnum::NO_AD);
            resp.set_msg("没有广告");
            return 0;
        }
        // 打乱
        // std::shuffle(g_typeMap[type].begin(), g_typeMap[type].end());

        // 随机取出广告
        vector<shared_ptr<Advertise>> result;
        generateRandomId(g_typeMap[type], num, result, type);
        for (auto &ad : result)
        {
            // size_t randVal = rand() % g_typeMap[type].size();
            // auto randId = generateRandomId(vec, uniqueSet);
            // if (randId == -1)
            // {
            //     LOG(ERROR) << "not enough ad object!!!";
            //     resp.set_err(errorEnum::NOT_ENOUGH_AD);
            //     resp.set_msg("广告数量不足");
            //     return 0;
            // }

            // auto ad = g_typeMap[type].at(randId);
            uint32_t id = ad->id;
            string uuid = ad->uuid;
            string imageUrl = ad->imageUrl;
            string url = ad->url;
            string content = ad->content;
            // uint32_t type = ad->type;

            ::ad_proto::AdInfo *info = resp.add_info();
            info->set_id(id);
            info->set_imageurl(imageUrl);
            info->set_url(url);
            info->set_content(content);
            info->set_type(type);
        }

        // auto randId = generateRandomId(vec, uniqueSet);
        if (result.size() < num)
        {
            LOG(ERROR) << "not enough ad object!!!";
            resp.set_err(errorEnum::NOT_ENOUGH_AD);
            resp.set_msg("广告数量不足");
            return 0;
        }
        resp.set_err(errorEnum::SUCCESS);
        resp.set_msg("获取广告成功");

        return 0;
    }

    // 获取广告时非重复
    void Ad::generateRandomId(vector<shared_ptr<Advertise>> &src, size_t rand_count, vector<shared_ptr<Advertise>> &result, uint32_t type)
    {
        priority_queue<shared_ptr<Advertise>> q;

        size_t len = src.size();
        if (rand_count >= len)
        {
            result = src;
            return;
        }

        /**
         * @brief 方案1：随机分配，公平
         *
         */
        // for (size_t i = 0; i < rand_count; i++)
        // {
        //     size_t j = rand() % len;
        //     auto tmp = src[i];
        //     src[i] = src[j];
        //     src[j] = tmp;
        // }

        /**
         * @brief Balance. 更适用于广告出价比较接近的场景
         * 方案2：res = 单位消耗/总金额，取res值最小的几个。
         * 尽量使得所有⼴告都保持匀速投放
         */
        // auto fun = [&](shared_ptr<Advertise> &a, shared_ptr<Advertise> &b)
        // {
        //     auto userA = getUser(a->uuid);
        //     auto userB = getUser(b->uuid);

        //     double Aamount = userA->amount;
        //     double Bamount = userB->amount;

        //     // return (CLICK_COST / Aamount) < (CLICK_COST / Bamount)
        //     return Aamount > Bamount;
        // };
        // std::sort(src.begin(), src.end(), fun);

        /**
         * @brief 方案3：贪心，出价高的先展示
         *
         *
         */
        // auto fun = [&](shared_ptr<Advertise> &a, shared_ptr<Advertise> &b)
        // {
        //     double priceA = getCostByADType(a->type);
        //     double priceB = getCostByADType(b->type);

        //     return priceA > priceB;
        // };
        // std::sort(src.begin(), src.end(), fun);

        /**
         * @brief
         * 方案4：res = 单位消耗/总金额，广告主可以动态调节总预算
         * 权衡函数：f(res) = 1 - e^(res - 1)
         * 缩放出价：出价 * fun(res)
         * 当
         */

        /**
         * @brief 方案5：权重分配
         *
         */
        int total = 0;
        for (auto &iter : g_weightMap[type])
        {
            total += iter.second;
        }
        // 随机
        std::set<uint32_t> s;
        int randVal = (rand() % total) + 1;
        // 取rand_count个
        for (size_t i = 0; i < rand_count; i++)
        {
            int tmpVal = 0;
            // 遍历广告列表
            for (auto &ptr : src)
            {
                // 如果没有取过
                if (!s.count(ptr->id))
                {
                    tmpVal += g_weightMap[type][ptr->id];
                    if (tmpVal >= randVal)
                    {
                        result.push_back(ptr);
                        g_weightMap[type][ptr->id] -= 128;
                        s.insert(ptr->id);

                        if (g_weightMap[type][ptr->id] <= 0)
                        {
                            g_weightMap[type][ptr->id] = 0;
                        }

                        break;
                    }
                }
            }
        }
        // 处理没随机到的
        for (auto &iter : g_weightMap[type])
        {
            // cout << "id: " << iter.first << "  weight: " << iter.second << endl;
            if (!s.count(iter.first))
            {
                if ((iter.second + 256) >= RAND_MAX / 2)
                {
                    iter.second = RAND_MAX / 2;
                    continue;
                }

                iter.second += 256;
            }
        }

        //---------------------
        // result.clear();
        // result.reserve(rand_count);
        // for (size_t i = 0; i < rand_count; i++)
        // {
        //     result.push_back(src[i]);
        // }

        return;
    }

    //获得用户
    user_ptr Ad::getUser(string uuid)
    {
        if (g_userMap.find(uuid) != g_userMap.end())
        {
            LOG(INFO) << "getUser: " << uuid << "g_userMap.size()" << g_userMap.size();
            return g_userMap[uuid];
        }
        else
        {
            initUser(uuid);
        }

        // 再次查找
        if (g_userMap.find(uuid) != g_userMap.end())
        {
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
        auto ret = MyDB::getInstance()->syncExecSQL(string(buf));
        if (ret.size())
        {
            auto m = ret[0];
            uint32_t id = strtoul(m["id"].c_str(), nullptr, 0);
            string uuid = m["uuid"];
            string url = m["url"];
            string imageUrl = m["imageUrl"];
            string content = m["content"];
            uint32_t type = strtoul(m["type"].c_str(), nullptr, 0);
            uint32_t status = strtoul(m["status"].c_str(), nullptr, 0);

            Advertise ad(id, uuid, imageUrl, url, content, type, status);
            auto ptr = make_shared<Advertise>(ad);
            g_adMap[id] = ptr;
            return g_adMap[id];
        }

        return nullptr;
    }

    // 将用户数据加载到内存
    bool Ad::initUser(string uuid)
    {
        std::lock_guard<std::mutex> init_lock(init_mtx);
        // if (g_AUMap.find(uuid) != g_AUMap.end() &&
        if (g_userMap.find(uuid) != g_userMap.end() &&
            g_countMap.find(uuid) != g_countMap.end())
        {
            return true;
        }
        // g_AUMap[uuid].clear();
        g_userMap[uuid].reset();
        g_userMap.erase(uuid);
        g_countMap[uuid].reset();
        g_countMap.erase(uuid);

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
        auto ret1 = MyDB::getInstance()->syncExecSQL(sql);
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
        ret1 = MyDB::getInstance()->syncExecSQL(sql);
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
        // ad_list list;

        ADUser user(uuid, username, password, phone, amount, welfare);
        user_ptr ptr = make_shared<ADUser>(user);
        g_userMap[uuid] = ptr;
        // g_AUMap[uuid] = list;
        // g_adMap[id] = ptr;

        // 统计类
        uint64_t clickNum = 0;
        uint64_t showNum = 0;
        uint64_t sellNum = 0;
        uint64_t visitNum = 0;
        uint64_t shopNum = 0;
        // char buf[2048];
        memset(buf, 0, 2048);
        sprintf(buf, "SELECT SUM(click), SUM(exposure) FROM adFlow WHERE uuid='%s';", uuid.c_str());
        ret1 = MyDB::getInstance()->syncExecSQL(string(buf));

        memset(buf, 0, 2048);
        sprintf(buf, "SELECT SUM(sell), SUM(visit), SUM(shop) FROM adAction WHERE uuid='%s';", uuid.c_str());
        auto ret2 = MyDB::getInstance()->syncExecSQL(string(buf));

        if (ret1.size())
        {
            clickNum = strtoull(ret1[0]["SUM(click)"].c_str(), nullptr, 0);
            showNum = strtoull(ret1[0]["SUM(exposure)"].c_str(), nullptr, 0);
        }
        if (ret2.size())
        {
            sellNum = strtoull(ret2[0]["SUM(sell)"].c_str(), nullptr, 0);
            visitNum = strtoull(ret2[0]["SUM(visit)"].c_str(), nullptr, 0);
            shopNum = strtoull(ret2[0]["SUM(shop)"].c_str(), nullptr, 0);
        }

        double costs = clickNum * CLICK_COST + (showNum / 1000.0) * MILLE_COST +
                       sellNum * SELL_COST + visitNum * VISIT_COST + shopNum * SHOP_COST;

        ADCount c(costs, clickNum, showNum, sellNum, visitNum, shopNum);
        auto c_ptr = make_shared<ADCount>(c);
        g_countMap[uuid] = c_ptr;

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
        auto ret = MyDB::getInstance()->syncExecSQL(string(buf));

        uint32_t id = strtoul(ret[0]["COUNT(id)"].c_str(), nullptr, 0);

        LOG(INFO) << "COUNT(id): " << id;
        return id + 1;
    }

    bool Ad::checkType(uint32_t type)
    {
        if (type == errorEnum::CLICK ||
            type == errorEnum::MILLE ||
            type == errorEnum::VISIT ||
            type == errorEnum::SHOP ||
            type == errorEnum::TIME ||
            type == errorEnum::SELL)
        {
            return true;
        }

        return false;
    }

    double Ad::getCostByADType(uint32_t type)
    {
        if (type == errorEnum::CLICK)
        {
            return CLICK_COST;
        }
        else if (type == errorEnum::MILLE)
        {
            return MILLE_COST / 1000;
        }
        else if (type == errorEnum::VISIT)
        {
            return VISIT_COST;
        }
        else if (type == errorEnum::SHOP)
        {
            return SHOP_COST;
        }
        else if (type == errorEnum::SELL)
        {
            return SELL_COST;
        }

        LOG(ERROR) << "unknow ad type: " << type;
        return 0;
    }
}