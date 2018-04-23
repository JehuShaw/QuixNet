/*
 * File:   MySQLDatabase.cpp
 * Author: Jehu Shaw
 *
 */

#include "DatabaseEnv.h"

#if defined(ENABLE_DATABASE_MYSQL)

#include "MySQLDatabase.h"

using namespace util;

namespace db {

MySQLDatabase::~MySQLDatabase()
{
}

MySQLDatabase::MySQLDatabase() : Database()
{
}

void MySQLDatabase::BeginTransaction(CAutoPointer<DatabaseConnection> conn)
{
	SendQuery(conn, "BEGIN;", false);
}

void MySQLDatabase::EndTransaction(CAutoPointer<DatabaseConnection> conn)
{
	SendQuery(conn, "COMMIT;", false);
}

void MySQLDatabase::RollbackTransaction(CAutoPointer<DatabaseConnection> conn)
{
	SendQuery(conn, "ROLLBACK;", false);
}

bool MySQLDatabase::Initialize(
	const char* szHostName,
	unsigned int nPort,
	const char* szUserName,
	const char* szPassword,
	const char* szDatabaseName,
	int32_t nConnectionSize)
{
	int32_t i;
	MYSQL * pMysql1 = NULL;
	MYSQL * pMysql2 = NULL;
	my_bool bMyTrue = true;

	if(nConnectionSize < 1) {
		return false;
	}

	OutputBasic("Connecting to `%s`, database `%s`...", szHostName, szDatabaseName);

	for(i = 0; i < m_nConnectionSize; ++i) {
		CAutoPointer<MySQLDatabaseConnection> conn(m_arrConnections[i]);
		if(conn.IsInvalid()) {
			continue;
		}
		mysql_close(conn->MySql);
	}
    delete [] m_arrConnections;
	m_arrConnections = new CAutoPointer<DatabaseConnection> [nConnectionSize];
	m_nConnectionSize = nConnectionSize;

	for(i = 0; i < nConnectionSize; ++i) {

		pMysql1 = mysql_init(NULL);
		if(pMysql1 == NULL) {
			continue;
		}

		if(mysql_options(pMysql1, MYSQL_SET_CHARSET_NAME, "utf8")) {
			OutputError("Could not set utf8 character set.");
		}

		pMysql2 = mysql_real_connect(pMysql1, szHostName, szUserName, szPassword,
			szDatabaseName, nPort, NULL, CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS);
		if(pMysql2 == NULL) {
			OutputError("Connection failed due to: `%s`", mysql_error(pMysql1));
			mysql_close(pMysql1);
			return false;
		}

		if (mysql_options(pMysql1, MYSQL_OPT_RECONNECT, &bMyTrue)) {
			OutputError("MYSQL_OPT_RECONNECT could not be set, connection drops may occur but will be counteracted.");
		}

		CAutoPointer<MySQLDatabaseConnection> pMysqlConnc(new MySQLDatabaseConnection);
		pMysqlConnc->MySql = pMysql2;
		m_arrConnections[i] = pMysqlConnc;

        ++m_nCurConCount;
	}

	Database::Initialize();
	return true;
}

void MySQLDatabase::Dispose()
{
	m_nCurConCount = 0;
	for(int32_t i = 0; i < m_nConnectionSize; ++i) {
		CAutoPointer<MySQLDatabaseConnection> conn(m_arrConnections[i]);
		if(conn.IsInvalid()) {
			continue;
		}
		mysql_close(conn->MySql);
	}
	delete [] m_arrConnections;
	m_arrConnections = NULL;
}

string MySQLDatabase::EscapeString(const string& esc) throw()
{
	CAutoPointer<MySQLDatabaseConnection> mysqlCon(GetFreeConnection());
	if(mysqlCon.IsInvalid()) {
		return string();
	}
	string ret;
	size_t nBufSize = esc.length() * 2 + 1;
	if(MAX_SQL_BUF_SIZE < nBufSize)
	{
		char* to = new char[nBufSize];
		unsigned long nEscLen = mysql_real_escape_string(
			mysqlCon->MySql, to, esc.c_str(), esc.length());
		if(0 == nEscLen)
		{
			ret = esc;
		} else {
			ret.assign(to, nEscLen);
		}
		delete [] to;
	}
	else
	{
		char a2[MAX_SQL_BUF_SIZE] = { 0 };
		unsigned long nEscLen = mysql_real_escape_string(
			mysqlCon->MySql, a2, esc.c_str(), esc.length());
		if(0 == nEscLen)
		{
			ret = esc;
		} else {
			ret.assign(a2, nEscLen);
		}
	}

	mysqlCon->busy.Unlock();

	return ret;
}

void MySQLDatabase::EscapeLongString(const char * str, uint32_t len, stringstream& out) throw()
{
	CAutoPointer<MySQLDatabaseConnection> mysqlCon(GetFreeConnection());
	if(mysqlCon.IsInvalid()) {
		return;
	}
	const static size_t nStackSize = 196608;
	size_t nBufSize = len * 2 + 1;
	if(nStackSize < nBufSize)
	{
		char* to = new char[nBufSize];
		unsigned long nEscLen = mysql_real_escape_string(
			mysqlCon->MySql, to, str, (unsigned long)len);
		if(0 == nEscLen)
		{
			out.write(str, (std::streamsize)len);
		} else {
			out.write(to, (std::streamsize)nEscLen);
		}
		delete [] to;
	}
	else 
	{
		char a2[nStackSize] = { 0 };
		unsigned long nEscLen = mysql_real_escape_string(
			mysqlCon->MySql, a2, str, (unsigned long)len);
		if(0 == nEscLen)
		{
			out.write(str, (std::streamsize)len);
		} else {
			out.write(a2, (std::streamsize)nEscLen);
		}
	}

	mysqlCon->busy.Unlock();
}

string MySQLDatabase::EscapeString(const char * esc, CAutoPointer<DatabaseConnection> con) throw()
{
	CAutoPointer<MySQLDatabaseConnection> mysqlCon(con);
	if(mysqlCon.IsInvalid()) {
		return string();
	}
	string ret;
	unsigned long nSrcLen = strlen(esc);
	size_t nBufSize = nSrcLen * 2 + 1;
	if(MAX_SQL_BUF_SIZE < nBufSize)
	{
		char* to = new char[nBufSize];
		unsigned long nEscLen = mysql_real_escape_string(
			mysqlCon->MySql, to, esc, nSrcLen);
		if(0 == nEscLen)
		{
			ret.assign(esc, nSrcLen);
		} else {
			ret.assign(to, nEscLen);
		}
		delete [] to;
	}
	else
	{
		char a2[MAX_SQL_BUF_SIZE] = { 0 };
		unsigned long nEscLen = mysql_real_escape_string(
			mysqlCon->MySql, a2, (char*)esc, nSrcLen);
		if(0 == nEscLen)
		{
			ret.assign(esc, nSrcLen);
		} else {
			ret.assign(a2, nEscLen);
		}
	}
	return ret;
}

void MySQLDatabase::Shutdown()
{
	EndThreads();
	mysql_library_end();
}

bool MySQLDatabase::SendQuery(CAutoPointer<DatabaseConnection>& con, const char* Sql, bool Self)
{
	//dunno what it does ...leaving untouched
    CAutoPointer<MySQLDatabaseConnection> myCon(con);
    if(myCon.IsInvalid())
	{
		return false; 
	}
	int result = mysql_query(myCon->MySql, Sql);
	if(0 == result)
	{
		return true;
	}
	else
	{
		if(Self == false && HandleError(myCon, mysql_errno(myCon->MySql)))
		{
			// Re-send the query, the connection was successful.
			// The true on the end will prevent an endless loop here, as it will
			// stop after sending the query twice.
			return SendQuery(con, Sql, true);
		}
		else
		{
            //printf("Sql query failed due to [%s], Query: [%s]", mysql_error(myCon->MySql), Sql);
			OutputError("Sql query failed due to [%s], Query: [%s]", mysql_error(myCon->MySql), Sql);
		}
	}
	return false;
}

bool MySQLDatabase::HandleError(CAutoPointer<MySQLDatabaseConnection>& con, uint32_t ErrorNumber)
{
	// Handle errors that should cause a reconnect to the Database.
	switch(ErrorNumber)
	{
	case 2006:  // Mysql server has gone away
	case 2008:  // Client ran out of memory
	case 2013:  // Lost connection to sql server during query
	case 2055:  // Lost connection to sql server - system error
		{
			// Let's instruct a reconnect to the db when we encounter these errors.
			return Reconnect(con);
		}break;
	}

	return false;
}

int MySQLDatabase::SelectDB(const char *db)
{
    if(m_nCurConCount < 1) {
        return 0;
    }
    int nResult(0);
    for(int32_t i = 0; i < m_nConnectionSize; ++i) {
        CAutoPointer<MySQLDatabaseConnection> myCon(m_arrConnections[i]);
        if(myCon.IsInvalid()) {
            continue;
        }
        int nRt = mysql_select_db(myCon->MySql, db);
        if(0 != nRt) {
            nResult = nRt;
        }
    }
    return nResult;
}

MySQLQueryResult::MySQLQueryResult(MYSQL_RES* res, MYSQL_FIELD *fields, uint32_t FieldCount, uint32_t RowCount)
	: QueryResult(FieldCount, RowCount), mResult(res), mField(fields)
{
}

MySQLQueryResult::~MySQLQueryResult()
{
	mysql_free_result(mResult);
}

bool MySQLQueryResult::NextRow()
{
	MYSQL_ROW row = mysql_fetch_row(mResult);
	if(row == NULL) {
		return false;
	}
	Field* arrRows = Fetch();
	if(NULL == arrRows) {
		return false;
	}
	for(uint32_t i = 0; i < m_nFieldCount; ++i) {
		arrRows[i].SetValue(row[i]);
        arrRows[i].SetField(mField[i].name);
    }
	return true;
}

Field* MySQLQueryResult::GetFieldByName(const char* szFieldName)
{
	Field* arrRows = Fetch();
	if(szFieldName && arrRows && mField) {
		for(int i = 0; i < (int)m_nFieldCount ;++i) {
			if(strcmp(mField[i].name, szFieldName) == 0) {
				return &(arrRows[i]);
			}
		}
	}
	return NULL;
}

CAutoPointer<QueryResult> MySQLDatabase::StoreQueryResult(CAutoPointer<DatabaseConnection>& con)
{
	CAutoPointer<MySQLDatabaseConnection> db(con);
    if(db.IsInvalid()) {
        return CAutoPointer<QueryResult>();
    }

	MYSQL_RES * pRes = mysql_store_result(db->MySql);
	if(NULL == pRes) {
		bool bVain = true;
		int32_t nRows = (int32_t)mysql_affected_rows(db->MySql);
		mysql_free_result(pRes);
		while(0 == mysql_next_result(db->MySql)) {
			pRes = mysql_store_result(db->MySql);
			if(NULL != pRes) {
				bVain = false;
				break;
			}
			nRows = (int32_t)mysql_affected_rows(db->MySql);
			mysql_free_result(pRes);
		}
		if(bVain) {
			if(mysql_field_count(db->MySql) == 0) {
				// query does not return data
				// (it was not a SELECT)
				if(nRows > -1) {
					return CAutoPointer<QueryResult>(
						new MySQLQueryResult(NULL, NULL, 0, nRows));
				}
			}
			return CAutoPointer<QueryResult>();
		}
	}

	int32_t nRows = (int32_t)mysql_affected_rows(db->MySql);
	uint32_t uFields = (uint32_t)mysql_field_count(db->MySql);
    MYSQL_FIELD *fields = mysql_fetch_fields(pRes);
	if(nRows < 1 || uFields == 0) {
		mysql_free_result(pRes);
		// delete left result set.
		while(0 == mysql_next_result(db->MySql)) {
			MYSQL_RES* res = mysql_store_result(db->MySql);
			mysql_free_result(res);
		}
		return CAutoPointer<QueryResult>();
	}

	CAutoPointer<MySQLQueryResult> res = CAutoPointer<MySQLQueryResult>(
		new MySQLQueryResult(pRes, fields, uFields, nRows));
	res->NextRow();
	// only return first result, delete left result set.
	while(0 == mysql_next_result(db->MySql)) {
		MYSQL_RES* res = mysql_store_result(db->MySql);
		mysql_free_result(res);
	}
	return res;
}

bool MySQLDatabase::Reconnect(CAutoPointer<MySQLDatabaseConnection>& conn)
{
	if(conn.IsInvalid() || NULL == conn->MySql) {
		assert(false);
		return false;
	}
	const char* szHost = conn->MySql->host;
	const char* szUser = conn->MySql->user;
	const char* szPwd = conn->MySql->passwd;
	const char* szDB = conn->MySql->db;
	unsigned int nPort = conn->MySql->port;

	MYSQL * temp, *temp2;

	temp = mysql_init(NULL);
	temp2 = mysql_real_connect(temp, szHost, szUser, szPwd, szDB,
		nPort, NULL, CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS);
	if(temp2 == NULL) {
		OutputError("Could not reconnect to database because of `%s`", mysql_error(temp));
		mysql_close(temp);
		return false;
	}

	if(conn->MySql != NULL) {
		mysql_close(conn->MySql);
	}
	conn->MySql = temp;
	return true;
}

} // end namespace db

#endif
