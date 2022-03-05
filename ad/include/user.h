#ifndef __USER_H__
#define __USER_H__
#include <iostream>

using namespace std;

namespace ad_namespace
{
    // 用户类，缓存
    class ADUser
    {
    public:
        // 用户基本信息
        std::string uuid;
        std::string username;
        uint64_t phone;
        double amount;
        double welfare;
        // double rtCost;                              // 实时消耗
        uint64_t lDt; //最后更新时间
        // std::unordered_map<uint64_t, ad_namespace::Advertise> adMap; // 广告id和广告映射

    public:
        ADUser(/* args */);
        ADUser(string uuid, string username, uint64_t phone, double amount, double welfare);
        ~ADUser();
        // typedef std::shared_ptr<ADUser> user_ptr;

    public:
        string getUuid();
    };

    //广告类
    class Advertise
    {
    public:
        uint64_t id;
        string url;
        string imageUrl;
        string content;
        double cost;  // 实时消耗
        uint64_t lDt; //最后更新时间

    public:
        Advertise(/* args */);
        Advertise(uint64_t id, double cost, string content);
        ~Advertise();
        // typedef std::shared_ptr<Advertise> ad_ptr;
    };

    // static void reflectUser(user_ptr user);
    // static bool isUserExist(user_ptr user);
}

#endif