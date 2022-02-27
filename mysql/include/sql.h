#pragma once
#include <iostream>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <queue>
using namespace std;

class MyDB
{
public:
	static MyDB *instance;
	static MyDB *getInstance(void);

public:
	bool connect(string host, int port, string user, string passwd, string database);						  //连接数据库
	bool setTable(string sql);																				  //创建表
	bool delTable(string tablename);																		  //删除表
	bool insertValues(string tablename, string values);														  //插入数据
	bool delValues(string tablename, string clounmname, string keyname);									  //删除数据
	bool searchValues(string tablename, int num);															  //查询数据
	bool updateValues(string tablename, string updatename, string values, string clounmname, string keyname); //修改数据
	void EXIT_ERROR(string msg);
	MYSQL_RES *searchValuesToRes(std::string sql);
	bool exeSQL(string sql);

public:
	MYSQL_ROW getRow(void);
	MYSQL_RES *getResult(void);
	MYSQL_FIELD *getField(void);

private:
	MyDB();
	~MyDB();

private:
	MYSQL *mysql;		//用于定义一个mysql对象，便于后续操作确定要操作的数据库是哪一个。
	MYSQL_ROW row;		//用于定义一个行对象，其内容含有一行的数据。
	MYSQL_RES *result;	//用于定义一个存储数据库检索信息结果的对象。
	MYSQL_FIELD *field; //用于定义一个存储字段信息的对象。
};