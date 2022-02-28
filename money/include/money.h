#ifndef __LOGIN_H__
#define __LOGIN_H__
#include <iostream>
#include <string>
#include <memory>
#include "money.pb.h"
#include "mysql.pb.h"

using namespace std;

namespace money_namespace
{
    class Money
    {
    public:
        /* data */
        float amount;
        float welfare;

    public:
        Money(/* args */);
        ~Money();
        int getFundInfo(money_proto::GetFundInfoReq &req, money_proto::GetFundInfoResp &resp); //获得所有信息
        int recharge(money_proto::RechargeReq &req, money_proto::RechargeResp &resp);          //充值
        int deduction(money_proto::DeductionReq &req, money_proto::DeductionResp &resp);       //扣费
    private:
        void invoke(mysql_proto::SaveReq &request, mysql_proto::SaveResp &response);
    };
}

#endif