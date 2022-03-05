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
        int costPerClick(ad_proto::CostPerClickReq &req, ad_proto::CostPerClickResp &resp);    //获得所有信息
        int costPerMille(ad_proto::CostPerMilleReq &req, ad_proto::CostPerMilleResp &resp);    //充值
        int costPerAction(ad_proto::CostPerActionReq &req, ad_proto::CostPerActionResp &resp); //扣费
        int putAdvertise(ad_proto::PutAdvertiseReq &req, ad_proto::PutAdvertiseResp &resp);    //广告投放
        int getAdInfo(ad_proto::GetAdInfoReq &req, ad_proto::GetAdInfoResp &resp);             // 获取广告信息

    public:
        int login(ad_proto::LoginReq &req, ad_proto::LoginResp &resp);        //登录逻辑
        int regist(ad_proto::RegisterReq &req, ad_proto::RegisterResp &resp); //注册逻辑

    public:
        int getFundInfo(ad_proto::GetFundInfoReq &req, ad_proto::GetFundInfoResp &resp); //获得所有信息
        int recharge(ad_proto::RechargeReq &req, ad_proto::RechargeResp &resp);          //充值
        int deduction(ad_proto::DeductionReq &req, ad_proto::DeductionResp &resp);       //扣费

    private:
        void invoke(mysql_proto::SaveReq &request, mysql_proto::SaveResp &response);
        void reflectUser(std::unordered_map<string, string> &data);
        std::shared_ptr<ad_namespace::ADUser> getUser(string uuid);
        std::shared_ptr<ad_namespace::ADUser> getAdList(string uuid);
        std::shared_ptr<ad_namespace::Advertise> getAdvertise(string uuid, uint64_t id);
        bool initUser(string uuid); // 将用户数据加载到内存
    };
}

#endif