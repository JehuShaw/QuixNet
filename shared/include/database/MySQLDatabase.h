/*
 * File:   MySQLDatabase.h
 * Author: Jehu Shaw
 *
 */

#ifndef MYSQLDATABASE_H
#define MYSQLDATABASE_H

#include <mysql.h>
#include "Database.h"

namespace db {

class SchemeDescriptor;
class dbFieldDescriptor;

struct MySQLDatabaseConnection : public DatabaseConnection
{
	MYSQL * MySql;
};

class SHARED_DLL_DECL MySQLDatabase : public Database
{
	friend class QueryThread;
	//friend class BatchQuery;
public:
	MySQLDatabase();
	~MySQLDatabase();

	bool Initialize(
		const char* szHostName,
		unsigned int nPort,
		const char* szUserName,
		const char* szPassword,
		const char* szDatabaseName,
		int32_t nConnectionSize);

	void Dispose();

	void Shutdown();

	std::string EscapeString(const std::string& esc) throw();
	void EscapeLongString(const char * str, uint32_t len, std::stringstream& out) throw();
	std::string EscapeString(const char * esc, util::CAutoPointer<DatabaseConnection> con) throw();

	bool SupportsReplaceInto() { return true; }
	bool SupportsTableLocking() { return true; }

	void BeginTransaction(util::CAutoPointer<DatabaseConnection> conn);
	void EndTransaction(util::CAutoPointer<DatabaseConnection> conn);
	void RollbackTransaction(util::CAutoPointer<DatabaseConnection> conn);

    int SelectDB(const char *db);

protected:

	bool HandleError(util::CAutoPointer<MySQLDatabaseConnection>& con, uint32_t ErrorNumber);
	bool SendQuery(util::CAutoPointer<DatabaseConnection>& con, const char* Sql, bool Self = false);

	bool Reconnect(util::CAutoPointer<MySQLDatabaseConnection>& conn);

	util::CAutoPointer<QueryResult> StoreQueryResult(util::CAutoPointer<DatabaseConnection>& con);

};

class SHARED_DLL_DECL MySQLQueryResult : public QueryResult, public util::PoolBase<MySQLQueryResult>
{
public:
	MySQLQueryResult(MYSQL_RES* res, MYSQL_FIELD *fields , uint32_t FieldCount, uint32_t RowCount);
	~MySQLQueryResult();

	bool NextRow();

	Field* GetFieldByName(const char* szFieldName);

protected:
	MYSQL_RES* mResult;
	MYSQL_FIELD* mField;
};

}

#endif /* MYSQLDATABASE_H */
