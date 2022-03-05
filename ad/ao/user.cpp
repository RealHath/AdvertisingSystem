#include "user.h"

namespace ad_namespace
{
    ADUser::ADUser() {}
    ADUser::ADUser(string uuid, string username, uint64_t phone, double amount, double welfare)
        : uuid(uuid), username(username), phone(phone), amount(amount), welfare(welfare)
    {
        this->lDt = time(NULL);
    }
    ADUser::~ADUser() {}

    string ADUser::getUuid()
    {
        return this->uuid;
    }

    //--------------------

    Advertise::Advertise() {}
    Advertise::Advertise(uint64_t id, double cost, string content)
        : id(id), cost(cost), content(content)
    {
        this->lDt = time(NULL);
    }
    Advertise::~Advertise() {}
}