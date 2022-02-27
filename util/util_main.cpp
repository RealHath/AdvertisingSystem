#include "common.h"
#include "test.pb.h"

int main()
{
    std::string data = genUUID();
    std::cout << data << std::endl;

    test::TestRequest req;
    test::TestRequest resp;
    req.set_name("姓名");
    req.set_num(2);
    std::string data2 = pb2json(req);
    std::cout << "pb2json:" << data2;

    json2pb(data2, resp);

    return 0;
}