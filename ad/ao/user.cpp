#include "user.h"
#include "sql.h"
#include "errorEnum.pb.h"

using namespace mysqlpp;

// std::unordered_map<errorEnum::ADType, std::string> g_ASMap;
// g_ASMap[errorEnum::CLICK] = ""

namespace ad_namespace
{
    ADUser::ADUser() {}
    ADUser::ADUser(string uuid, string username, string password, uint64_t phone, double amount, double welfare)
        : uuid(uuid), username(username), password(password), phone(phone), amount(amount), welfare(welfare)
    {
        this->lDt = time(NULL);
    }
    ADUser::~ADUser() {}

    string ADUser::getUuid()
    {
        return this->uuid;
    }

    // 更新用户资金
    void ADUser::updateMoney(double increment)
    {
        auto conn = MyDB::getInstance()->getConnection();
        mysqlpp::Transaction
            tran(*conn, Transaction::IsolationLevel::repeatable_read,
                 Transaction::IsolationScope::session);

        char buf[2048];
        sprintf(buf, "UPDATE money SET amount=%lf, welfare=%lf, updateTime=%lu WHERE uuid='%s';",
                amount, welfare, time(NULL), uuid.c_str());
        string sql(buf);
        MyDB::getInstance()->execSQL(sql);

        memset(buf, 0, 2048);
        sprintf(buf, "INSERT INTO recharge(id,uuid,increment,updateTime) VALUES(null,'%s',%lf,%lu);",
                uuid.c_str(), increment, time(NULL));
        MyDB::getInstance()->execSQL(string(buf));
        tran.commit();
    }

    // 插入用户
    void ADUser::insertUser()
    {
        auto conn = MyDB::getInstance()->getConnection();
        mysqlpp::Transaction
            tran(*conn, Transaction::IsolationLevel::repeatable_read,
                 Transaction::IsolationScope::session);

        char buf[2048];
        sprintf(buf, "INSERT INTO user(id,username,password,phone,uuid,registTime) VALUES(null,'%s','%s',%lu,'%s',%lu);",
                username.c_str(), password.c_str(), phone, uuid.c_str(), time(NULL));
        string sql(buf);
        MyDB::getInstance()->execSQL(sql);
        memset(buf, 0, 2048);
        sprintf(buf, "INSERT INTO money(id,uuid,amount,welfare,updateTime) VALUES(null,'%s',%lf,%lf,%lu);",
                uuid.c_str(), amount, welfare, time(NULL));
        sql = string(buf);
        MyDB::getInstance()->execSQL(sql);
        tran.commit();
    }

    //--------------------

    Advertise::Advertise()
    {
    }
    Advertise::Advertise(uint32_t id, string uuid, string imageUrl, string url, string content, uint32_t type)
        : id(id), uuid(uuid), imageUrl(imageUrl), url(url), content(content), type(type)
    {
        this->lDt = time(NULL);
    }
    Advertise::~Advertise() {}

    void Advertise::insertAd()
    {
        auto conn = MyDB::getInstance()->getConnection();
        mysqlpp::Transaction
            tran(*conn, Transaction::IsolationLevel::repeatable_read,
                 Transaction::IsolationScope::session);
        char buf[2048];
        sprintf(buf, "INSERT INTO ad(id,uuid,type,imageUrl,url,content,updateTime) VALUES(%u,'%s',%u,'%s','%s','%s',%lu);",
                id, uuid.c_str(), type, imageUrl.c_str(), url.c_str(), content.c_str(), time(NULL));
        string sql = string(buf);
        MyDB::getInstance()->execSQL(sql);

        memset(buf, 0, 2048);
        if (type == errorEnum::ADType::CLICK)
        {
            sprintf(buf, "INSERT INTO adFlow(id,uuid,click,exposure,updateTime) VALUES(%u,'%s',0,0,%lu);",
                    id, uuid.c_str(), time(NULL));
            sql = string(buf);
        }
        else if (type == errorEnum::ADType::MILLE)
        {
            sprintf(buf, "INSERT INTO adFlow(id,uuid,click,exposure,updateTime) VALUES(%u,'%s',0,0,%lu);",
                    id, uuid.c_str(), time(NULL));
            sql = string(buf);
        }
        else if (type == errorEnum::ADType::VISIT)
        {
            sprintf(buf, "INSERT INTO adFlow(id,uuid,visit,shop,sell,updateTime) VALUES(%u,'%s',0,0,0,%lu);",
                    id, uuid.c_str(), time(NULL));
            sql = string(buf);
        }
        else if (type == errorEnum::ADType::SHOP)
        {
            sprintf(buf, "INSERT INTO adFlow(id,uuid,visit,shop,sell,updateTime) VALUES(%u,'%s',0,0,0,%lu);",
                    id, uuid.c_str(), time(NULL));
            sql = string(buf);
        }
        else if (type == errorEnum::ADType::SELL)
        {
            sprintf(buf, "INSERT INTO adFlow(id,uuid,visit,shop,sell,updateTime) VALUES(%u,'%s',0,0,0,%lu);",
                    id, uuid.c_str(), time(NULL));
            sql = string(buf);
        }
        else if (type == errorEnum::ADType::TIME)
        {
            sprintf(buf, "INSERT INTO adFlow(id,uuid,startTime,endTime) VALUES(%u,'%s',%lu,%lu);",
                    id, uuid.c_str(), time(NULL), time(NULL) + 3600);
            sql = string(buf);
        }
        else
        {
            sql.clear();
        }
        if (sql.size())
        {
            MyDB::getInstance()->execSQL(sql);
        }

        tran.commit();
    }

    void Advertise::updateCost()
    {
        char buf[2048];
        string sql;
        if (type == errorEnum::ADType::CLICK)
        {
            sprintf(buf, "UPDATE adFlow SET 'click' = 'click'+1,'updateTime' = %lu WHERE id=%u AND uuid='%s';",
                    time(NULL), id, uuid.c_str());
            sql = string(buf);
        }
        else if (type == errorEnum::ADType::MILLE)
        {
            sprintf(buf, "UPDATE adFlow SET 'exposure' = 'exposure'+1,'updateTime' = %lu WHERE id=%u AND uuid='%s';",
                    time(NULL), id, uuid.c_str());
            sql = string(buf);
        }
        else if (type == errorEnum::ADType::VISIT)
        {
            sprintf(buf, "UPDATE adAction SET 'visit' = 'visit'+1,'updateTime' = %lu WHERE id=%u AND uuid='%s';",
                    time(NULL), id, uuid.c_str());
            sql = string(buf);
        }
        else if (type == errorEnum::ADType::SHOP)
        {
            sprintf(buf, "UPDATE adAction SET 'shop' = 'shop'+1,'updateTime' = %lu WHERE id=%u AND uuid='%s';",
                    time(NULL), id, uuid.c_str());
            sql = string(buf);
        }
        else if (type == errorEnum::ADType::SELL)
        {
            sprintf(buf, "UPDATE adAction SET 'sell' = 'sell'+1,'updateTime' = %lu WHERE id=%u AND uuid='%s';",
                    time(NULL), id, uuid.c_str());
            sql = string(buf);
        }
        // else if (type == errorEnum::ADType::TIME)
        // {
        //     sprintf(buf, "UPDATE adFlow SET 'click' = 'click'+1 WHERE id=%u AND uuid='%s';",
        //             id, uuid.c_str());
        //     sql = string(buf);
        // }
        else
        {
            sql.clear();
        }

        if (sql.size())
        {
            MyDB::getInstance()->execSQL(sql);
        }
    }
}