#ifndef __USER_H__
#define __USER_H__
#include <iostream>
#include <memory>
#include <unordered_map>

using namespace std;

std::unordered_map<std::string, user_ptr> g_userMap;
typedef std::shared_ptr<ad_namespace::ADUser> user_ptr;
typedef std::shared_ptr<ad_namespace::Advertise> ad_ptr;

namespace ad_namespace
{
    // 用户类，缓存
    class ADUser
    {
    private:
        // 用户基本信息
        std::string uuid;
        uint64_t phone;
        double amount;
        double welfare;
        double rtCost;                              // 实时消耗
        uint64_t lDt;                               //最后更新时间
        std::unordered_map<uint64_t, ad_ptr> adMap; // 广告id和广告映射

    public:
        User(/* args */);
        ~User();
    };

    //广告类
    class Advertise
    {
    private:
        uint64_t id;
        string url;
        string imageUrl;
        string content;
        double cost;  // 实时消耗
        uint64_t lDt; //最后更新时间

    public:
        Advertise(/* args */);
        ~Advertise();
    };

}

#endif