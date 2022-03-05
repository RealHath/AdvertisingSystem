#pragma once
#include <iostream>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <vector>
#include <tuple>
#include <mysql++/mysql++.h>
#include <unordered_map>
using namespace std;

class MyDB
{
public:
	static MyDB *instance;
	static MyDB *getInstance(void);

public:
	//连接数据库
	bool connect(string host, int port, string user, string passwd, string database);
	tuple<int, vector<vector<string>>> execSQL(string sql);
	vector<std::unordered_map<string, string>> execute(string sql);
	mysqlpp::Connection *getConnection();

private:
	MyDB();
	~MyDB();

private:
	MYSQL *mysql;		//用于定义一个mysql对象，便于后续操作确定要操作的数据库是哪一个。
	MYSQL_ROW row;		//用于定义一个行对象，其内容含有一行的数据。
	MYSQL_RES *result;	//用于定义一个存储数据库检索信息结果的对象。
	MYSQL_FIELD *field; //用于定义一个存储字段信息的对象。
	mysqlpp::Connection *conn;
	mysqlpp::Query *query;
};