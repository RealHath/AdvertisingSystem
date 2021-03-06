#include "sql.h"
#include <butil/logging.h>
#include "errorEnum.pb.h"
#include <mutex>

std::mutex read_mtx;
std::mutex write_mtx;

using namespace std;

MyDB::MyDB(/* args */)
{
	// mysql = mysql_init(nullptr);
	write_conn = new mysqlpp::Connection(false);
	read_conn = new mysqlpp::Connection(false);
}

MyDB::~MyDB()
{
	// if (mysql != nullptr)
	// {
	// 	mysql_close(mysql);
	// }
	// if (result != nullptr)
	// {
	// 	mysql_free_result(result);
	// }
	// conn->shutdown();
	write_conn->disconnect();
	read_conn->disconnect();
	delete read_conn;
	delete write_conn;
}

MyDB *MyDB::getInstance()
{
	static MyDB instance;
	return &instance;
}
//连接数据库
bool MyDB::connect(std::string host, int port, std::string user, std::string passwd, std::string database)
{
	// mysql = mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str(), database.c_str(), port, nullptr, 0);
	write_conn->connect(database.c_str(), host.c_str(), user.c_str(), passwd.c_str(), port);
	read_conn->connect(database.c_str(), host.c_str(), user.c_str(), passwd.c_str(), port);
	if (read_conn->connected() == false || write_conn->connected() == false)
	{
		std::cout << "connect database failed!" << std::endl;
		return false;
	}

	return true;
}

// 用于查询
vector<std::unordered_map<string, string>> MyDB::syncExecSQL(string sql = "PING")
{
	std::lock_guard<std::mutex> g_lock(read_mtx);
	if (sql == "PING")
	{
		LOG(INFO) << "Mysql Ping!!!!!";
		read_conn->ping();
		return {};
	}
	LOG(INFO) << "syncExecSQL: " << sql;
	vector<std::unordered_map<string, string>> res;
	// sql = "SELECT * FROM user WHERE uuid='94f06243-7feb-443f-b4d9-a7d1f820171e';";
	auto query = read_conn->query();
	query << sql;
	mysqlpp::StoreQueryResult ares = query.store();
	// LOG(INFO) << "ares.num_rows() = " << ares.num_rows();
	for (size_t i = 0; i < ares.num_rows(); i++)
	{
		// std::cout << "your data" << std::endl;
		// std::cout << "name: " << ares[i]["name"] << "\tid: " << ares[i]["id"] << "\tstatus: " << ares[i]["status"] << std::endl;

		std::unordered_map<string, string> tmp;

		for (size_t j = 0; j < ares[i].size(); j++)
		{
			string field_name = ares.field_name(j);
			tmp[field_name] = string(ares[i][field_name.c_str()]);
			cout << " " << field_name.c_str() << ": " << ares[i][field_name.c_str()];
		}
		cout << endl;
		res.push_back(tmp);
	}

	if (res.size() == 0)
	{
		LOG(WARNING) << "result empty!! sql: " << sql;
	}

	return res;
}

// 用于修改
void MyDB::asyncExecSQL(string sql = "PING")
{
	std::lock_guard<std::mutex> g_lock(write_mtx);
	if (sql == "PING")
	{
		LOG(INFO) << "Mysql Ping!!!!!";
		write_conn->ping();
		return;
	}
	LOG(INFO) << "asyncExecSQL: " << sql;
	auto query = write_conn->query();
	query << sql;
	bool flag = query.exec();

	if (!flag)
	{
		LOG(ERROR) << "asyncExecSQL failed!! sql: " << sql;
	}
}

mysqlpp::Connection *MyDB::getConnection()
{
	return this->write_conn;
}