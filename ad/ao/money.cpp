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