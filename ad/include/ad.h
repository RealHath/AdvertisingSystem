#ifndef __LOGIN_H__
#define __LOGIN_H__
#include <iostream>
#include <string>
#include <memory>
#include "ad.pb.h"
#include "mysql.pb.h"

using namespace std;

namespace ad_namespace
{
    class ADUser;
    class Advertise;
    class Ad
    {
    public:
    public:
        Ad(/* args */);
        ~Ad();
        int costPerClick(ad_proto::CostPerClickReq &req, ad_proto::CostPerClickResp &resp); //获得所有信息
        int costPerMille(ad_proto::CostPerMilleReq &req, ad_proto::CostPerMilleResp &resp); //充值
        int costPerVisit(ad_proto::CostPerVisitReq &req, ad_proto::CostPerVisitResp &resp); //扣费
        int costPerShop(ad_proto::CostPerShopReq &req, ad_proto::CostPerShopResp &resp);    //扣费
        int costPerTime(ad_proto::CostPerTimeReq &req, ad_proto::CostPerTimeResp &resp);    //扣费
        int costPerSell(ad_proto::CostPerSellReq &req, ad_proto::CostPerSellResp &resp);    //扣费
        int putAdvertise(ad_proto::PutAdvertiseReq &req, ad_proto::PutAdvertiseResp &resp); //广告投放
        int getAdInfo(ad_proto::GetAdInfoReq &req, ad_proto::GetAdInfoResp &resp);          // 获取广告信息

    public:
        int login(ad_proto::LoginReq &req, ad_proto::LoginResp &resp);        //登录逻辑
        int regist(ad_proto::RegisterReq &req, ad_proto::RegisterResp &resp); //注册逻辑

    public:
        int getFundInfo(ad_proto::GetFundInfoReq &req, ad_proto::GetFundInfoResp &resp); //获得所有信息
        int recharge(ad_proto::RechargeReq &req, ad_proto::RechargeResp &resp);          //充值
        int deduction(ad_proto::DeductionReq &req, ad_proto::DeductionResp &resp);       //扣费
        int getCount(ad_proto::GetCountReq &req, ad_proto::GetCountResp &resp);          // 获取统计

    private:
        // void invoke(mysql_proto::SaveReq &request, mysql_proto::SaveResp &response);
        // void reflectUser(std::unordered_map<string, string> &data);
        std::shared_ptr<ad_namespace::ADUser> getUser(string uuid);
        // std::shared_ptr<ad_namespace::ADUser> getAdList(string uuid);
        std::shared_ptr<ad_namespace::Advertise> getAdvertise(uint32_t id);
        bool initUser(string uuid); // 将用户数据加载到内存
        uint32_t generateAdId();    // 生成广告id
        // void init();
    };
}

typedef std::shared_ptr<ad_namespace::ADUser> user_ptr;
typedef std::shared_ptr<ad_namespace::Advertise> ad_ptr;
typedef std::unordered_map<uint32_t, ad_ptr> ad_list;

static std::unordered_map<std::string, user_ptr> g_userMap;    // 用户映射表
static std::unordered_map<uint32_t, ad_ptr> g_adMap;           // 用户广告映射表
static std::unordered_map<std::string, ad_list> g_AUMap;       // 用户广告映射表
static std::unordered_map<uint32_t, vector<ad_ptr>> g_typeMap; // 类型广告映射表

#endif