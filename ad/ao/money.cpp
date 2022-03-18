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

extern std::shared_ptr<ad_namespace::ADUser> getUser(string uuid);
extern std::shared_ptr<ad_namespace::ADUser> getAdList(string uuid);
extern std::shared_ptr<ad_namespace::Advertise> getAdvertise(string uuid, uint64_t id);
extern bool initUser(string uuid); // 将用户数据加载到内存

/////////---------------------------------------------------
int Ad::getFundInfo(ad_proto::GetFundInfoReq &req, ad_proto::GetFundInfoResp &resp)
{
    // 1. 处理入参
    string uuid = req.uuid();
    auto user = getUser(uuid);

    // 如果内存没有
    if (user == nullptr)
    {
        // 初始化
        bool flag = initUser(uuid);
        if (!flag)
        {
            // 如果数据库没有
            resp.set_err(errorEnum::NO_ACCOUNT);
            resp.set_msg("没有此账号");
            return 0;
        }
    }
    user = getUser(uuid);
    double amount = user->amount;
    double welfare = user->welfare;
    resp.set_err(errorEnum::SUCCESS);
    resp.set_msg("查找成功");
    resp.set_uuid(uuid);
    resp.set_amount(amount);
    resp.set_welfare(welfare);
    return 0;
}
int Ad::recharge(ad_proto::RechargeReq &req, ad_proto::RechargeResp &resp)
{
    // 1. 处理入参
    string uuid = req.uuid();
    double money = req.money();

    if (money <= 0)
    {
        resp.set_err(errorEnum::NUMBER_ERROR);
        resp.set_msg("充值金额错误");
        return 0;
    }

    auto user = getUser(uuid);

    // 如果内存没有
    if (user == nullptr)
    {
        // 初始化
        bool flag = initUser(uuid);
        if (!flag)
        {
            // 如果数据库没有
            resp.set_err(errorEnum::NO_ACCOUNT);
            resp.set_msg("没有此账号");
            return 0;
        }
    }
    user = getUser(uuid);
    user->amount += money;
    user->updateMoney(money);

    double amount = user->amount;
    double welfare = user->welfare;
    resp.set_err(errorEnum::SUCCESS);
    resp.set_msg("充值成功");
    resp.set_uuid(uuid);
    resp.set_amount(amount);
    resp.set_welfare(welfare);
    return 0;
}
int Ad::deduction(ad_proto::DeductionReq &req, ad_proto::DeductionResp &resp)
{
    // 1. 处理入参
    string uuid = req.uuid();
    double money = req.money();

    auto user = getUser(uuid);

    // 如果内存没有
    if (user == nullptr)
    {
        // 初始化
        bool flag = initUser(uuid);
        if (!flag)
        {
            // 如果数据库没有
            resp.set_err(errorEnum::NO_ACCOUNT);
            resp.set_msg("没有此账号");
            return 0;
        }
    }
    user = getUser(uuid);
    user->amount -= money;
    user->updateMoney(money * -1);

    double amount = user->amount;
    double welfare = user->welfare;
    resp.set_err(errorEnum::SUCCESS);
    resp.set_msg("扣费成功");
    resp.set_uuid(uuid);
    resp.set_amount(amount);
    resp.set_welfare(welfare);
    return 0;
}

int Ad::getCount(ad_proto::GetCountReq &req, ad_proto::GetCountResp &resp)
{
    // 1. 处理入参
    string uuid = req.uuid();

    if (true || g_countMap.find(uuid) == g_countMap.end())
    {
        char buf[2048];
        memset(buf, 0, 2048);
        sprintf(buf, "SELECT SUM(click), SUM(exposure) FROM adFlow WHERE uuid='%s';", uuid.c_str());
        auto ret1 = MyDB::getInstance()->syncExecSQL(string(buf));

        memset(buf, 0, 2048);
        sprintf(buf, "SELECT SUM(sell), SUM(visit), SUM(shop) FROM adAction WHERE uuid='%s';", uuid.c_str());
        auto ret2 = MyDB::getInstance()->syncExecSQL(string(buf));

        // memset(buf, 0, 2048);
        // sprintf(buf, "SELECT * FROM adTime WHERE uuid='%s';", uuid.c_str());
        // auto ret3 = MyDB::getInstance()->syncExecSQL(string(buf));

        uint64_t clickNum = 0;
        uint64_t showNum = 0;
        uint64_t sellNum = 0;
        uint64_t visitNum = 0;
        uint64_t shopNum = 0;
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

        // LOG(INFO) << clickNum << showNum << sellNum << visitNum << shopNum << costs;

        ADCount c(costs, clickNum, showNum, sellNum, visitNum, shopNum);
        auto c_ptr = make_shared<ADCount>(c);
        g_countMap[uuid] = c_ptr;
    }

    auto count = g_countMap[uuid];

    resp.set_err(0);
    resp.set_msg("查询成功");
    resp.set_costs(count->costs);
    resp.set_clicknum(count->clickNum);
    resp.set_shownum(count->showNum);
    resp.set_sellnum(count->sellNum);
    resp.set_visitnum(count->visitNum);
    resp.set_shopnum(count->shopNum);

    return 0;
}