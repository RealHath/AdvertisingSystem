#include <butil/logging.h>

#include "login.h"
#include "mysqlSave.h"
#include "common.h"

using namespace std;

namespace login_namespace
{
    Login::Login()
    {
    }
    Login::~Login()
    {
    }
    int Login::login(login_proto::LoginReq &req, login_proto::LoginResp &resp)
    {
        LOG(INFO) << "调用 login";
    }
    int Login::regist(login_proto::RegisterReq &req, login_proto::RegisterResp &resp)
    {
        string username = req.username();
        string password = req.password();
        uint64_t phone = req.phone();

        // 先查库有没有数据

        // 没有再注册
        string uuid = genUUID();

        resp.set_err(0);
        resp.set_uuid(uuid);
    }
}