#ifndef __USER_H__
#define __USER_H__
#include <iostream>
#include <memory>
#include <unordered_map>

#include "errorEnum.pb.h"

using namespace std;

namespace ad_namespace
{

    //广告类
    class Advertise
    {
    public:
        uint32_t id;
        string uuid;
        uint32_t type;
        string url;
        string imageUrl;
        string content;
        uint64_t lDt; //最后更新时间
        uint32_t status;

    public:
        Advertise(/* args */);
        Advertise(uint32_t id, string uuid, string imageUrl, string url, string content, uint32_t type, uint32_t status);
        ~Advertise();
        // typedef std::shared_ptr<Advertise> ad_ptr;

    public:
        void insertAd();
        void updateCost();
    };

    // 用户类，缓存
    class ADUser
    {
    public:
        // 用户基本信息
        std::string uuid;
        std::string username;
        std::string password;
        uint64_t phone;
        double amount;
        double welfare;
        // double rtCost;                              // 实时消耗
        uint64_t lDt;                                                                 //最后更新时间
        std::unordered_map<uint64_t, std::shared_ptr<ad_namespace::Advertise>> adMap; // 广告id和广告映射

    public:
        ADUser(/* args */);
        ADUser(string uuid, string username, string password, uint64_t phone, double amount, double welfare);
        ~ADUser();
        // typedef std::shared_ptr<ADUser> user_ptr;

    public:
        string getUuid();
        void updateMoney(double increment);
        void insertUser();
    };

    // 统计类
    class Count
    {
    public:
        double costs;
        uint64_t clickNum; // 点击
        uint64_t showNum;  // 曝光
        uint64_t sellNum;  // 销售
        uint64_t visitNum; // 浏览
        uint64_t shopNum;  // 加购
    };
}

#endif