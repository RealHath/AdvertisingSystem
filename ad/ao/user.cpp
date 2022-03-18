#include "user.h"
#include "sql.h"
#include <butil/logging.h>

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

        try
        {
            char buf[2048];
            sprintf(buf, "UPDATE money SET amount=%lf, welfare=%lf, updateTime=%lu WHERE uuid='%s';",
                    amount, welfare, time(NULL), uuid.c_str());
            string sql(buf);
            MyDB::getInstance()->asyncExecSQL(sql);

            memset(buf, 0, 2048);
            sprintf(buf, "INSERT INTO recharge(id,uuid,increment,updateTime) VALUES(null,'%s',%lf,%lu);",
                    uuid.c_str(), increment, time(NULL));
            MyDB::getInstance()->asyncExecSQL(string(buf));
            tran.commit();
        }
        catch (std::exception &e)
        {
            tran.rollback();
        }
    }

    // 插入用户
    void ADUser::insertUser()
    {
        auto conn = MyDB::getInstance()->getConnection();
        mysqlpp::Transaction
            tran(*conn, Transaction::IsolationLevel::repeatable_read,
                 Transaction::IsolationScope::session);

        try
        {
            char buf[2048];
            sprintf(buf, "INSERT INTO user(id,username,password,phone,uuid,registTime) VALUES(null,'%s','%s',%lu,'%s',%lu);",
                    username.c_str(), password.c_str(), phone, uuid.c_str(), time(NULL));
            string sql(buf);
            MyDB::getInstance()->asyncExecSQL(sql);
            memset(buf, 0, 2048);
            sprintf(buf, "INSERT INTO money(id,uuid,amount,welfare,updateTime) VALUES(null,'%s',%lf,%lf,%lu);",
                    uuid.c_str(), amount, welfare, time(NULL));
            sql = string(buf);
            MyDB::getInstance()->asyncExecSQL(sql);
            tran.commit();
        }
        catch (std::exception &e)
        {
            LOG(ERROR) << "updateMoney rollback";
            tran.rollback();
        }
    }

    //--------------------

    Advertise::Advertise()
    {
    }
    Advertise::Advertise(uint32_t id, string uuid, string imageUrl, string url, string content, uint32_t type, uint32_t status = errorEnum::NOT_ADUIT)
        : id(id), uuid(uuid), imageUrl(imageUrl), url(url), content(content), type(type), status(status)
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

        try
        {
            char buf[2048];
            sprintf(buf, "INSERT INTO ad(id,uuid,type,imageUrl,url,content,updateTime,status) VALUES(%u,'%s',%u,'%s','%s','%s',%lu,%u);",
                    id, uuid.c_str(), type, imageUrl.c_str(), url.c_str(), content.c_str(), time(NULL), errorEnum::NOT_ADUIT);
            string sql = string(buf);
            MyDB::getInstance()->asyncExecSQL(sql);

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
                sprintf(buf, "INSERT INTO adFlow(id,uuid,time,isEnd) VALUES(%u,'%s',%u,%u);",
                        id, uuid.c_str(), 3600, 0);
                sql = string(buf);
            }
            else
            {
                sql.clear();
            }
            if (sql.size())
            {
                MyDB::getInstance()->asyncExecSQL(sql);
            }

            tran.commit();
        }
        catch (std::exception &e)
        {
            LOG(ERROR) << "insertAd rollback";
            tran.rollback();
        }
    }

    void Advertise::updateCost()
    {
        auto conn = MyDB::getInstance()->getConnection();
        mysqlpp::Transaction
            tran(*conn, Transaction::IsolationLevel::repeatable_read,
                 Transaction::IsolationScope::session);

        try
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
            else if (type == errorEnum::ADType::TIME)
            {
                sprintf(buf, "UPDATE adFlow SET 'isEnd' = 2 WHERE id=%u AND uuid='%s';",
                        id, uuid.c_str());
                sql = string(buf);
            }
            else
            {
                sql.clear();
            }

            if (sql.size())
            {
                MyDB::getInstance()->asyncExecSQL(sql);
            }

            tran.commit();
        }
        catch (std::exception &e)
        {
            LOG(ERROR) << "updateCost rollback";
            tran.rollback();
        }
    }

    //-------------------------

    ADCount::ADCount(/* args */) {}

    ADCount::~ADCount() {}
    ADCount::ADCount(double costs, uint64_t clickNum, uint64_t showNum, uint64_t sellNum, uint64_t visitNum, uint64_t shopNum)
        : costs(costs), clickNum(clickNum), showNum(showNum), sellNum(sellNum), visitNum(visitNum), shopNum(shopNum)
    {
    }
}