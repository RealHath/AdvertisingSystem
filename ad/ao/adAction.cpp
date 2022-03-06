
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

// typedef std::shared_ptr<ad_namespace::ADUser> user_ptr;
// typedef std::shared_ptr<ad_namespace::Advertise> ad_ptr;
// typedef std::unordered_map<uint64_t, ad_ptr> ad_list;
// extern std::unordered_map<std::string, user_ptr> g_userMap; // 用户映射表
// extern std::unordered_map<std::string, ad_list> g_adMap;    // 用户广告映射表

extern std::shared_ptr<ad_namespace::ADUser> getUser(string uuid);
extern std::shared_ptr<ad_namespace::ADUser> getAdList(string uuid);
extern std::shared_ptr<ad_namespace::Advertise> getAdvertise(string uuid, uint64_t id);
extern bool initUser(string uuid); // 将用户数据加载到内存

int Ad::costPerVisit(ad_proto::CostPerVisitReq &req, ad_proto::CostPerVisitResp &resp)
{
    // 1. 处理入参
    uint64_t id = req.id();
    

    resp.set_err(errorEnum::SUCCESS);
    resp.set_msg("CPA");
    return 0;
}
int Ad::costPerShop(ad_proto::CostPerShopReq &req, ad_proto::CostPerShopResp &resp)
{

    resp.set_err(errorEnum::SUCCESS);
    resp.set_msg("CPA");
    return 0;
}