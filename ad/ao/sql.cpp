#include "sql.h"
#include <butil/logging.h>
#include "errorEnum.pb.h"
#include <mutex>

std::mutex g_mtx;

using namespace std;

MyDB::MyDB(/* args */)
{
	// mysql = mysql_init(nullptr);
	conn = new mysqlpp::Connection(false);
}

MyDB::~MyDB()
{
	if (mysql != nullptr)
	{
		mysql_close(mysql);
	}
	if (result != nullptr)
	{
		mysql_free_result(result);
	}
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
	conn->connect(database.c_str(), host.c_str(), user.c_str(), passwd.c_str(), port);
	if (mysql == nullptr && conn->connected() == false)
	{
		std::cout << "connect database failed!" << std::endl;
		return false;
	}
	else
	{
		return true;
	}
}

// 第一个返回值是状态码，第二个参数是查询值
tuple<int, vector<vector<string>>> MyDB::execSQL(string sql = "PING")
{
	std::lock_guard<std::mutex> g_lock(g_mtx);
	if (sql == "PING")
	{
		LOG(INFO) << "Mysql Ping!!!!!";
		mysql_ping(mysql);
		return make_tuple(0, vector<vector<string>>());
	}
	LOG(INFO) << "SQL: " << sql;

	// 结果
	vector<vector<string>> res;
	vector<vector<string>> res2;
	// mysql_query()执行成功返回0,执行失败返回非0值。
	if (mysql_query(mysql, sql.c_str()))
	{
		LOG(ERROR) << "Query Error: " << mysql_error(mysql);
		return make_tuple(errorEnum::MYSQL_QUERY_ERR, res2);
	}
	else // 查询成功
	{
		result = mysql_store_result(mysql); //获取结果集
		if (result)							// 返回了结果集
		{
			int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
			int num_rows = mysql_num_rows(result);	   //获取结果集中总共的行数

			res.resize(num_rows);

			for (int i = 0; i < num_rows; i++) //输出每一行
			{
				//获取下一行数据
				row = mysql_fetch_row(result);
				if (row == nullptr)
					break;

				vector<string> tmp;
				for (int j = 0; j < num_fields; j++) //输出每一字段
				{
					cout << string(row[j]) << endl;
					tmp.push_back(string(row[j]));
				}
				res.push_back(tmp);
			}
		}
		else // result==NULL
		{
			if (mysql_field_count(mysql) == 0) //代表执行的是update,insert,delete类的非查询语句
			{
				// (it was not a SELECT)
				int num_rows = mysql_affected_rows(mysql); //返回update,insert,delete影响的行数
			}
			else // error
			{
				LOG(ERROR) << "Get result error: " << mysql_error(mysql);
				return make_tuple(errorEnum::MYSQL_UPDATE_ERR, res2);
			}
		}
	}

	// 清除空

	if (res.size())
	{
		for (size_t i = 0; i < res.size(); ++i)
		{
			if (res[i].size())
			{
				vector<string> tmp;
				for (size_t j = 0; j < res[i].size(); ++j)
				{
					// if (res[i][j].size())
					// {
					tmp.push_back(res[i][j]);
					// }
				}
				res2.push_back(tmp);
			}
		}
	}

	return make_tuple(errorEnum::SUCCESS, res2);
}

vector<std::unordered_map<string, string>> MyDB::execute(string sql = "PING")
{
	std::lock_guard<std::mutex> g_lock(g_mtx);
	if (sql == "PING")
	{
		LOG(INFO) << "Mysql Ping!!!!!";
		conn->ping();
		return {};
	}
	LOG(INFO) << "SQL: " << sql;
	vector<std::unordered_map<string, string>> res;
	// sql = "SELECT * FROM user WHERE uuid='94f06243-7feb-443f-b4d9-a7d1f820171e';";
	auto query = conn->query();
	query << sql;
	mysqlpp::StoreQueryResult ares = query.store();
	// LOG(INFO) << "ares.num_rows() = " << ares.num_rows();
	for (size_t i = 0; i < ares.num_rows(); i++)
	{
		std::cout << "your data" << std::endl;
		// std::cout << "name: " << ares[i]["name"] << "\tid: " << ares[i]["id"] << "\tstatus: " << ares[i]["status"] << std::endl;

		std::unordered_map<string, string> tmp;

		for (size_t j = 0; j < ares[i].size(); j++)
		{
			string field_name = ares.field_name(j);
			tmp[field_name] = string(ares[i][field_name.c_str()]);
			cout << field_name.c_str() << ": " << ares[i][field_name.c_str()] << endl;
		}
		res.push_back(tmp);
	}

	return res;
}

mysqlpp::Connection *MyDB::getConnection()
{
	return this->conn;
}