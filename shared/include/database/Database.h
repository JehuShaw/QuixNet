/*
 * File:   Database.h
 * Author: Jehu Shaw
 *
 */

#ifndef _DATABASE_H
#define _DATABASE_H

#include <string>
#include "CircleQueue.h"
#include "CallBackDB.h"
#include "CThreads.h"
#include "AutoPointer.h"
#include "Field.h"
#include "SpinEvent.h"

namespace db {

using namespace std;
class QueryResult;
//class QueryThread;
class Database;

#define QUERY_RESULT_DEFAULT_FIELD_SIZE 32
#define QUERY_STRING_DEFAULT_SIZE 256
#define MAX_SQL_BUF_SIZE 16384

struct DatabaseConnection
{
	DatabaseConnection() : busy() {}
	virtual ~DatabaseConnection() {}
	thd::CCriticalSection busy;
};

class SHARED_DLL_DECL QueryStrBuffer
{
public:
	QueryStrBuffer() {
		m_query = m_defBuf;
		m_query[0] = 0;
	}
	~QueryStrBuffer() {
		Clear();
	}
	QueryStrBuffer(const QueryStrBuffer& orig) {
		int nSize = (int)strlen(orig.m_query) + 1;
		if(nSize > QUERY_STRING_DEFAULT_SIZE) {
			m_query = new char[nSize];
		} else {
			m_query = m_defBuf;
		}
		memcpy(m_query, orig.m_query, nSize);
		m_query[nSize - 1] = 0;
	}

	QueryStrBuffer& operator = (const QueryStrBuffer& right) {
		Clear();
		int nSize = (int)strlen(right.m_query) + 1;
		if(nSize > QUERY_STRING_DEFAULT_SIZE) {
			m_query = new char[nSize];
		} else {
			m_query = m_defBuf;
		}
		memcpy(m_query, right.m_query, nSize);
		m_query[nSize - 1] = 0;
		return *this;
	}

	INLINE void Clear() {
		if(m_defBuf != m_query) {
			delete [] m_query;
			m_query = m_defBuf;
		}
	}

	char* GetBuffer(int size) {
		Clear();
		if(size > QUERY_STRING_DEFAULT_SIZE) {
			m_query = new char[size];
		} else {
			m_query = m_defBuf;
		}
		return m_query;
	}

	INLINE const char* GetStr() const {
		return m_query;
	}


private:
	char* m_query;
	char m_defBuf[QUERY_STRING_DEFAULT_SIZE];
};

struct SHARED_DLL_DECL AsyncQueryResult
{
	util::CAutoPointer<QueryResult> result;
	QueryStrBuffer query;
};

class SHARED_DLL_DECL AsyncQueryBase
{
public:
	virtual ~AsyncQueryBase() { }

	virtual bool IsQueryEmpty() const = 0;
	virtual int GetQuerySize() const = 0;
	virtual util::CAutoPointer<QueryResult> GetQueryResult(int index) const = 0;

protected:
	friend class Database;
	virtual bool IsTransaction() const = 0;
	virtual void Invoke(void) = 0;
	virtual void SetQueryResult(int index, util::CAutoPointer<db::QueryResult> result) = 0;
	virtual const char* GetQueryString(int index) const = 0;
	virtual void ResetQueryString() = 0;
};

class SHARED_DLL_DECL AsyncQueryCb : public AsyncQueryBase
{
public:
	AsyncQueryCb(util::CAutoPointer<SQLCallbackBase> f) : m_pFunc(f), m_bTransaction(false) {}
	AsyncQueryCb(util::CAutoPointer<SQLCallbackBase> f, bool bTrans) : m_pFunc(f), m_bTransaction(bTrans) {}
	~AsyncQueryCb() { if(!m_arrQueries.empty()){ m_arrQueries.clear(); } }

	int AddQuery(const char * format, ...)
	{
		char query[MAX_SQL_BUF_SIZE];
		va_list vlist;
		va_start(vlist, format);
		vsnprintf(query, MAX_SQL_BUF_SIZE, format, vlist);
		va_end(vlist);

		query[MAX_SQL_BUF_SIZE-1] = 0;

		AsyncQueryResult res;
		int nSize = (int)strlen(query) + 1;
		char* pBuf = res.query.GetBuffer(nSize);
		memcpy(pBuf, query, nSize);
		int nIdx = m_arrQueries.size();
		m_arrQueries.push_back(res);
		return nIdx;
	}

	int AddQueryNA(const char * str)
	{
		AsyncQueryResult res;
		int nSize = (int)strlen(str) + 1;
		char* pBuf = res.query.GetBuffer(nSize);
		memcpy(pBuf, str, nSize);
		int nIdx = m_arrQueries.size();
		m_arrQueries.push_back(res);
		return nIdx;
	}

	int AddQueryStr(const string& str)
	{
		AsyncQueryResult res;
		int nSize = (int)str.length() + 1;
		char* pBuf = res.query.GetBuffer(nSize);
		memcpy(pBuf, str.c_str(), nSize);
		int nIdx = m_arrQueries.size();
		m_arrQueries.push_back(res);
		return nIdx;
	}

	virtual bool IsQueryEmpty() const {
		return m_arrQueries.empty();
	}

	virtual int GetQuerySize() const {
		return (int)m_arrQueries.size();
	}

	virtual util::CAutoPointer<QueryResult> GetQueryResult(int index) const {
		return m_arrQueries[index].result;
	}

protected:
	util::CAutoPointer<SQLCallbackBase> m_pFunc;
	vector<AsyncQueryResult> m_arrQueries;
	bool m_bTransaction;

	virtual bool IsTransaction() const {
		return m_bTransaction;
	}

	virtual void SetQueryResult(int index, util::CAutoPointer<QueryResult> result) {
		m_arrQueries[index].result = result;
	}

	virtual const char* GetQueryString(int index) const {
		return m_arrQueries[index].query.GetStr();
	}

	virtual void ResetQueryString() {
		for(int i = 0; i < (int)m_arrQueries.size(); ++i) {
			m_arrQueries[i].query.Clear();
		}
	}

	virtual void Invoke(void) {
		if(m_pFunc.IsInvalid()) {
			return;
		}
		m_pFunc->Invoke(m_arrQueries);
	}
};

class SHARED_DLL_DECL AsyncQueryWait : public AsyncQueryBase
{
public:
	AsyncQueryWait() : m_bTransaction(false) { m_waitEvent.Suspend(); }
	AsyncQueryWait(bool bTrans) : m_bTransaction(bTrans) { m_waitEvent.Suspend(); }
	~AsyncQueryWait() { Reset(); }

	int AddQuery(const char * format, ...)
	{
		char query[MAX_SQL_BUF_SIZE];
		va_list vlist;
		va_start(vlist, format);
		vsnprintf(query, MAX_SQL_BUF_SIZE, format, vlist);
		va_end(vlist);

		query[MAX_SQL_BUF_SIZE-1] = 0;

		AsyncQueryResult res;
		int nSize = (int)strlen(query) + 1;
		char* pBuf = res.query.GetBuffer(nSize);
		memcpy(pBuf, query, nSize);
		int nIdx = m_vecQueries.size();
		m_vecQueries.push_back(res);
		return nIdx;
	}

	int AddQueryNA(const char * str)
	{
		AsyncQueryResult res;
		int nSize = (int)strlen(str) + 1;
		char* pBuf = res.query.GetBuffer(nSize);
		memcpy(pBuf, str, nSize);
		int nIdx = m_vecQueries.size();
		m_vecQueries.push_back(res);
		return nIdx;
	}

	int AddQueryStr(const string& str)
	{
		AsyncQueryResult res;
		int nSize = (int)str.length() + 1;
		char* pBuf = res.query.GetBuffer(nSize);
		memcpy(pBuf, str.c_str(), nSize);
		int nIdx = m_vecQueries.size();
		m_vecQueries.push_back(res);
		return nIdx;
	}

	virtual bool IsQueryEmpty() const {
		return m_vecQueries.empty();
	}

	virtual int GetQuerySize() const {
		return (int)m_vecQueries.size();
	}

	virtual util::CAutoPointer<QueryResult> GetQueryResult(int index) const {
		return m_vecQueries[index].result;
	}

	void Wait(void) {
		m_waitEvent.Wait();
	}

	void Reset(void) {
		if(!m_vecQueries.empty()){
			m_vecQueries.clear();
		}
		m_waitEvent.Suspend();
	}

protected:
	thd::CSpinEvent m_waitEvent;
	vector<AsyncQueryResult> m_vecQueries;
	bool m_bTransaction;

	virtual bool IsTransaction() const {
		return m_bTransaction;
	}

	virtual void SetQueryResult(int index, util::CAutoPointer<QueryResult> result) {
		m_vecQueries[index].result = result;
	}

	virtual const char* GetQueryString(int index) const {
		return m_vecQueries[index].query.GetStr();
	}

	virtual void ResetQueryString() {
		for(int i = 0; i < (int)m_vecQueries.size(); ++i) {
			m_vecQueries[i].query.Clear();
		}
	}

	virtual void Invoke(void) {
		m_waitEvent.Resume();
	}
};

class SHARED_DLL_DECL Database : public thd::CThread
{
public:
	Database();
	virtual ~Database();

	/************************************************************************/
	/* Thread Stuff                                                         */
	/************************************************************************/
	bool Run();

	/************************************************************************/
	/* Virtual Functions                                                    */
	/************************************************************************/
	virtual bool Initialize(
		const char* szHostName,
		unsigned int nPort,
		const char* szUserName,
		const char* szPassword,
		const char* szDatabaseName,
		int32_t nConnectionSize) = 0;

	virtual void Dispose() = 0;

	virtual void Shutdown() = 0;

	virtual util::CAutoPointer<QueryResult> Query(const char* QueryString, ...) throw();
	virtual util::CAutoPointer<QueryResult> QueryNA(const char* QueryString) throw();
	virtual util::CAutoPointer<QueryResult> FQuery(const char* QueryString, util::CAutoPointer<DatabaseConnection> con);
	virtual bool FWaitExecute(const char* QueryString, util::CAutoPointer<DatabaseConnection> con);
	virtual bool WaitExecute(const char* QueryString, ...) throw();//Wait For Request Completion
	virtual bool WaitExecuteNA(const char* QueryString) throw();//Wait For Request Completion
	virtual bool Execute(const char* QueryString, ...);
	virtual bool ExecuteNA(const char* QueryString);

	INLINE const uint32_t GetQueueSize() { return m_queExecute.Size() + m_queQuery.Size(); }

	virtual string EscapeString(const string& esc) = 0;
	virtual void EscapeLongString(const char* str, uint32_t len, stringstream& out) = 0;
	virtual string EscapeString(const char* esc, util::CAutoPointer<DatabaseConnection> con) = 0;

	void WaitBatchQuery(util::CAutoPointer<AsyncQueryCb> aqcb) throw();
	void EndThreads();

	util::CAutoPointer<DatabaseConnection> GetFreeConnection() throw();

	void AddAsyncQuery(util::CAutoPointer<AsyncQueryBase> aq);

	static util::CAutoPointer<Database> CreateDatabaseInterface();
	static void CleanupLibs();

	virtual bool SupportsReplaceInto() = 0;
	virtual bool SupportsTableLocking() = 0;

	virtual void BeginTransaction(util::CAutoPointer<DatabaseConnection> conn) = 0;
	virtual void EndTransaction(util::CAutoPointer<DatabaseConnection> conn) = 0;
	virtual void RollbackTransaction(util::CAutoPointer<DatabaseConnection> conn) = 0;

    virtual int SelectDB(const char* db) = 0;

protected:

	void Initialize();

	// actual query function
	virtual bool SendQuery(util::CAutoPointer<DatabaseConnection>& con, const char* Sql, bool Self) = 0;
	virtual util::CAutoPointer<QueryResult> StoreQueryResult(util::CAutoPointer<DatabaseConnection>& con) = 0;

	void PerformAsyncQuery(util::CAutoPointer<AsyncQueryBase>& pAq, util::CAutoPointer<DatabaseConnection> ccon) throw();
	////////////////////////////////
	INLINE int32_t GetMaxThdSize() const {
		return m_nConnectionSize;
	}
	void TriggerRun();

	thd::CCircleQueue<util::CAutoPointer<AsyncQueryBase> > m_queQuery;
	thd::CCircleQueue<QueryStrBuffer> m_queExecute;
	////////////////////////////////////////////////////////
	util::CAutoPointer<DatabaseConnection> * m_arrConnections;
	int32_t m_nConnectionSize;
    int32_t m_nCurConCount;

	volatile int32_t m_nCurThdCount;
};

class SHARED_DLL_DECL QueryResult
{
public:
	QueryResult(uint32_t fields, uint32_t rows) : m_nFieldCount(fields), m_nRowCount(rows) {
		if(m_nFieldCount > QUERY_RESULT_DEFAULT_FIELD_SIZE) {
			m_arrCurrentRow = new Field[m_nFieldCount];
		} else {
			m_arrCurrentRow = m_arrDefaultFields;
		}
	}
	QueryResult(const QueryResult& orig) : m_nFieldCount(orig.m_nFieldCount), m_nRowCount(orig.m_nRowCount) {
		if(m_nFieldCount > QUERY_RESULT_DEFAULT_FIELD_SIZE) {
			m_arrCurrentRow = new Field[m_nFieldCount];
		} else {
			m_arrCurrentRow = m_arrDefaultFields;
		}
		for(unsigned int i = 0; i < m_nFieldCount; ++i) {
			m_arrCurrentRow[i] = orig.m_arrCurrentRow[i];
		}
	}
	virtual ~QueryResult() {
		if(m_arrDefaultFields != m_arrCurrentRow) {
			delete [] m_arrCurrentRow;
			m_arrCurrentRow = NULL;
		}
	}

	QueryResult& operator = (const QueryResult& right) {
		if(m_arrDefaultFields != m_arrCurrentRow) {
			delete [] m_arrCurrentRow;
			m_arrCurrentRow = NULL;
		}
		m_nFieldCount = right.m_nFieldCount;
		m_nRowCount = right.m_nRowCount;
		if(m_nFieldCount > QUERY_RESULT_DEFAULT_FIELD_SIZE) {
			m_arrCurrentRow = new Field[m_nFieldCount];
		} else {
			m_arrCurrentRow = m_arrDefaultFields;
		}
		for(unsigned int i = 0; i < m_nFieldCount; ++i) {
			m_arrCurrentRow[i] = right.m_arrCurrentRow[i];
		}
		return *this;
	}

	virtual bool NextRow() { return false; }

	INLINE Field* Fetch() { return m_arrCurrentRow; }
	INLINE uint32_t GetFieldCount() const { return m_nFieldCount; }
	INLINE uint32_t GetRowCount() const { return m_nRowCount; }
    virtual Field* GetFieldByName(const char* szFieldName) { return NULL; }

protected:
	uint32_t m_nFieldCount;
	uint32_t m_nRowCount;

private:
    Field* m_arrCurrentRow;
	Field m_arrDefaultFields[QUERY_RESULT_DEFAULT_FIELD_SIZE];
};

}

#endif
