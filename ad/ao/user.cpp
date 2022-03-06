#include "user.h"
#include "sql.h"

using namespace mysqlpp;

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
    void ADUser::updateMoney()
    {
        auto conn = MyDB::getInstance()->getConnection();
        mysqlpp::Transaction
            tran(*conn, Transaction::IsolationLevel::repeatable_read,
                 Transaction::IsolationScope::session);

        char buf[2048];
        sprintf(buf, "UPDATE money SET amount=%lf, welfare=%lf, updateTime=%lu WHERE uuid='%s';",
                amount, welfare, time(NULL), uuid.c_str());
        string sql(buf);
        MyDB::getInstance()->execute(sql);
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
        MyDB::getInstance()->execute(sql);
        memset(buf, 0, 2048);
        sprintf(buf, "INSERT INTO money(id,uuid,amount,welfare,updateTime) VALUES(null,'%s',%lf,%lf,%lu);",
                uuid.c_str(), amount, welfare, time(NULL));
        sql = string(buf);
        MyDB::getInstance()->execute(sql);
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
        MyDB::getInstance()->execute(sql);
        tran.commit();
    }
}