#include "sql.h"
#include <butil/logging.h>
#include "errorEnum.pb.h"

using namespace std;

MyDB::MyDB(/* args */)
{
	mysql = mysql_init(nullptr);
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
	mysql = mysql_real_connect(mysql, host.c_str(), user.c_str(), passwd.c_str(), database.c_str(), port, nullptr, 0);
	if (mysql == nullptr)
	{
		std::cout << "connect database failed!" << std::endl;
		return false;
	}
	else
	{
		return true;
	}
}

//插入数据
bool MyDB::insertValues(std::string tablename, string values) { return false; }
//删除数据
bool MyDB::delValues(std::string tablename, std::string clounmname, std::string keyname) { return false; }
//查询数据
bool MyDB::searchValues(std::string sql, int num)
{
	LOG(INFO) << "sql:" << sql;
	int res = mysql_query(mysql, sql.c_str());
	if (res == 0)
	{
		// std::cout << "serchValues successed!" << std::endl;
		// return true;
	}
	else
	{
		LOG(ERROR) << "serchValues failed!";
		// return false;
	}
	result = mysql_store_result(mysql);
	if (result) // 返回了结果集
	{
		int num_fields = mysql_num_fields(result); //获取结果集中总共的字段数，即列数
		int num_rows = mysql_num_rows(result);	   //获取结果集中总共的行数
		for (int i = 0; i < num_rows; i++)		   //输出每一行
		{
			//获取下一行数据
			row = mysql_fetch_row(result);
			if (row < 0)
				break;

			// for (int j = 0; j < num_fields; j++) //输出每一字段
			// {
			// 	std::cout << row[j] << "\t\t";
			// }
			// std::cout << std::endl;
		}
		return true;
	}
	return false;
}
//修改数据
bool MyDB::updateValues(std::string tablename, std::string updatename, std::string values, std::string clounmname, std::string keyname)
{
	return false;
}

//查询数据
MYSQL_RES *MyDB::searchValuesToRes(std::string sql)
{
	int res = mysql_query(mysql, sql.c_str());
	if (res == 0)
	{
		std::cout << "serchValues successed!" << std::endl;
		// return true;
	}
	else
	{
		std::cout << "serchValues failed!" << std::endl;
		return nullptr;
	}
	result = mysql_store_result(mysql);
	if (result->row_count > 0)
		return result;
	else
		return nullptr;
}

void MyDB::EXIT_ERROR(string msg)
{
}

// 第一个返回值是状态码，第二个参数是查询值
tuple<int, vector<vector<string>>> MyDB::execSQL(string sql)
{
	// 结果
	vector<vector<string>> res;
	// mysql_query()执行成功返回0,执行失败返回非0值。
	if (mysql_query(mysql, sql.c_str()))
	{
		LOG(ERROR) << "Query Error: " << mysql_error(mysql);
		return make_tuple(errorEnum::MYSQL_QUERY_ERR, res);
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
				if (row < 0)
					break;

				vector<string> tmp(num_fields);
				for (int j = 0; j < num_fields; j++) //输出每一字段
				{
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
				return make_tuple(errorEnum::MYSQL_UPDATE_ERR, res);
			}
		}
	}

	return make_tuple(errorEnum::SUCCESS, res);
}