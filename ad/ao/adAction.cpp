
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
// #include "mysql.pb.h"
#include "user.h"
#include "const.h"

using namespace std;
using namespace ad_namespace;

// typedef std::shared_ptr<ad_namespace::ADUser> user_ptr;
// typedef std::shared_ptr<ad_namespace::Advertise> ad_ptr;
// typedef std::unordered_map<uint64_t, ad_ptr> ad_list;
// extern std::unordered_map<std::string, user_ptr> g_userMap; // 用户映射表
// extern std::unordered_map<std::string, ad_list> g_AUMap;    // 用户广告映射表

extern std::shared_ptr<ad_namespace::ADUser> getUser(string uuid);
extern std::shared_ptr<ad_namespace::ADUser> getAdList(string uuid);
extern std::shared_ptr<ad_namespace::Advertise> getAdvertise(string uuid, uint64_t id);
extern bool initUser(string uuid); // 将用户数据加载到内存

int Ad::costPerVisit(ad_proto::CostPerVisitReq &req, ad_proto::CostPerVisitResp &resp)
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

    if (ad->type != errorEnum::VISIT)
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

    int a = rand() % 20 - 10;
    double cost = VISIT_COST + a / 100.0;

    // g_countMap[ad->uuid]->countAdd(ad->type);
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
    resp.set_msg("CPA_VISIT");
    return 0;
}
int Ad::costPerShop(ad_proto::CostPerShopReq &req, ad_proto::CostPerShopResp &resp)
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

    if (ad->type != errorEnum::SHOP)
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

    int a = rand() % 70 - 30;
    double cost = SHOP_COST + a / 100.0;

    // g_countMap[ad->uuid]->countAdd(ad->type);
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
    resp.set_msg("CPA_SHOP");
    return 0;
}

int Ad::costPerSell(ad_proto::CostPerSellReq &req, ad_proto::CostPerSellResp &resp)
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

    if (ad->type != errorEnum::SELL)
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

    int a = rand() % 10 - 5;
    double cost = SELL_COST + a;

    // g_countMap[ad->uuid]->countAdd(ad->type);
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
    resp.set_msg("CPS");
    return 0;
}
int Ad::costPerTime(ad_proto::CostPerTimeReq &req, ad_proto::CostPerTimeResp &resp)
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

    auto user = getUser(ad->uuid);
    if (user == nullptr)
    {
        initUser(ad->uuid);
        user = getUser(ad->uuid);
    }

    // g_countMap[ad->uuid]->countAdd(ad->type);
    user->amount -= TIME_COST;
    user->updateMoney(TIME_COST * -1);
    ad->updateCost();

    // if (user->amount < cost)
    // {
    //     user->changeAdStatus(errorEnum::NOT_ADUIT, ad->type);
    // g_typeMap[ad->type].clear();
    // }

    resp.set_err(errorEnum::SUCCESS);
    resp.set_msg("CPT");
    return 0;
}