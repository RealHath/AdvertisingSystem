
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

    user->amount -= VISIT_COST;
    user->updateMoney(VISIT_COST * -1);
    ad->updateCost();

    resp.set_err(errorEnum::SUCCESS);
    resp.set_msg("CPA_VISIT");
    return 0;
}
int Ad::costPerShop(ad_proto::CostPerShopReq &req, ad_proto::CostPerShopResp &resp)
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

    user->amount -= SHOP_COST;
    user->updateMoney(SHOP_COST * -1);
    ad->updateCost();

    resp.set_err(errorEnum::SUCCESS);
    resp.set_msg("CPA_SHOP");
    return 0;
}

int Ad::costPerSell(ad_proto::CostPerSellReq &req, ad_proto::CostPerSellResp &resp)
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

    user->amount -= SELL_COST;
    user->updateMoney(SELL_COST * -1);
    ad->updateCost();

    resp.set_err(errorEnum::SUCCESS);
    resp.set_msg("CPS");
    return 0;
}
int Ad::costPerTime(ad_proto::CostPerTimeReq &req, ad_proto::CostPerTimeResp &resp)
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

    user->amount -= TIME_COST;
    user->updateMoney(TIME_COST * -1);
    ad->updateCost();

    resp.set_err(errorEnum::SUCCESS);
    resp.set_msg("CPT");
    return 0;
}