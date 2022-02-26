#include <butil/logging.h>

#include "login.h"
#include "mysqlSave.h"

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
        string phone = req.phone();

        // 先查库有没有数据

        // 没有再注册
        string uuid;

        resp.set_err(0);
        resp.set_uuid()
    }
}