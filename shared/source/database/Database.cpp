/*
 * File:   Database.cpp
 * Author: Jehu Shaw
 *
 */

#include "DatabaseEnv.h"

using namespace util;
using namespace thd;

namespace db {

Database::Database() : CThread(), m_queQuery(), m_queExecute()
	, m_arrConnections(NULL), m_nConnectionSize(-1), m_nCurConCount(0)
	, m_nCurThdCount(0)
{
#if COMPILER == COMPILER_MICROSOFT
	BUILD_BUG_ON(sizeof(int32_t) < sizeof(long));
#endif
}

Database::~Database()
{
   EndThreads();
}

void Database::Initialize()
{
	SetThreadState(THREADSTATE_FREE);
}

CAutoPointer<DatabaseConnection> Database::GetFreeConnection() throw()
{
    if(m_nCurConCount < 1 || m_nConnectionSize < 1) {
        return CAutoPointer<DatabaseConnection>();
    }
	int32_t i = 0;
	for(;;)
	{
		CAutoPointer<DatabaseConnection> con(m_arrConnections[ ((i++) % m_nConnectionSize) ]);
		if(!con.IsInvalid() && con->busy.TryLock()) {
			return con;
		}
	}

	// shouldn't be reached
	return CAutoPointer<DatabaseConnection>();
}

// Use this when we request data that can return a value (not async)
CAutoPointer<QueryResult> Database::Query(const char* QueryString, ...) throw()
{
	char sql[MAX_SQL_BUF_SIZE];
	va_list vlist;
	va_start(vlist, QueryString);
	vsnprintf(sql, MAX_SQL_BUF_SIZE, QueryString, vlist);
	va_end(vlist);

	sql[MAX_SQL_BUF_SIZE-1] = 0;

	// Send the query
	CAutoPointer<QueryResult> qResult;
	CAutoPointer<DatabaseConnection> con(GetFreeConnection());

	if(SendQuery(con, sql, false)) {
		qResult = StoreQueryResult(con);
    }

    if(!con.IsInvalid()) {
	    con->busy.Unlock();
    }
	return qResult;
}

CAutoPointer<QueryResult> Database::QueryNA(const char* QueryString) throw()
{
	// Send the query
	CAutoPointer<QueryResult> qResult;
	CAutoPointer<DatabaseConnection> con(GetFreeConnection());

	if(SendQuery(con, QueryString, false)) {
		qResult = StoreQueryResult(con);
	}

    if(!con.IsInvalid()) {
	    con->busy.Unlock();
    }
	return qResult;
}

CAutoPointer<QueryResult> Database::FQuery(const char * QueryString, CAutoPointer<DatabaseConnection> con)
{
	// Send the query
	CAutoPointer<QueryResult> qResult;
	if(SendQuery(con, QueryString, false)) {
		qResult = StoreQueryResult(con);
	}
	return qResult;
}

bool Database::FWaitExecute(const char * QueryString, CAutoPointer<DatabaseConnection> con)
{
	// Send the query
	return SendQuery(con, QueryString, false);
}

void Database::PerformAsyncQuery(CAutoPointer<AsyncQueryBase>& pAq, CAutoPointer<DatabaseConnection> ccon) throw()
{
    if(pAq.IsInvalid()) {
        return;
    }

	if(pAq->IsQueryEmpty()) {
		return;
	}

	CAutoPointer<DatabaseConnection> con(ccon);
	if(ccon.IsInvalid()) {
		con = GetFreeConnection();
	}

	if(pAq->IsTransaction()) {
		BeginTransaction(con);
	}

	int nQuerySize = pAq->GetQuerySize();
	for(int i = 0; i < nQuerySize; ++i) {
		if(SendQuery(con, pAq->GetQueryString(i), false)) {
			pAq->SetQueryResult(i, StoreQueryResult(con));
		}
	}
	pAq->ResetQueryString();

	if(pAq->IsTransaction()) {
		EndTransaction(con);
	}

	if(ccon.IsInvalid() && !con.IsInvalid()) {
		con->busy.Unlock();
    }

	pAq->Invoke();
}
// Use this when we do not have a result. ex: INSERT into SQL 1
bool Database::Execute(const char* QueryString, ...)
{
	char query[MAX_SQL_BUF_SIZE];

	va_list vlist;
	va_start(vlist, QueryString);
	vsnprintf(query, MAX_SQL_BUF_SIZE, QueryString, vlist);
	va_end(vlist);

	query[MAX_SQL_BUF_SIZE-1] = 0;

	CThreadState threadState = GetThreadState();

	if(THREADSTATE_TERMINATE == threadState) {
		return true;
	} else if(THREADSTATE_AWAITING == threadState) {
		return WaitExecuteNA(query);
	}

	QueryStrBuffer* pQSB = m_queExecute.WriteLock();
	int nSize = (int)strlen(query) + 1;
	char* pBuf = pQSB->GetBuffer(nSize);
	memcpy(pBuf, query, nSize);
	m_queExecute.WriteUnlock();

	// Spawn Database thread
	TriggerRun();
	return true;
}

bool Database::ExecuteNA(const char* QueryString)
{
	CThreadState threadState = GetThreadState();

	if(THREADSTATE_TERMINATE == threadState) {
		return true;
	} else if(THREADSTATE_AWAITING == threadState) {
		return WaitExecuteNA(QueryString);
	}

	QueryStrBuffer* pQSB = m_queExecute.WriteLock();
	int nSize = (int)strlen(QueryString) + 1;
	char* pBuf = pQSB->GetBuffer(nSize);
	memcpy(pBuf, QueryString, nSize);
	m_queExecute.WriteUnlock();

	// Spawn Database thread
	TriggerRun();
	return true;
}

// Wait till the other queries are done, then execute
bool Database::WaitExecute(const char* QueryString, ...) throw()
{
	char sql[MAX_SQL_BUF_SIZE];
	va_list vlist;
	va_start(vlist, QueryString);
	vsnprintf(sql, MAX_SQL_BUF_SIZE, QueryString, vlist);
	va_end(vlist);

	sql[MAX_SQL_BUF_SIZE-1] = 0;

	CAutoPointer<DatabaseConnection> con(GetFreeConnection());
	bool Result = SendQuery(con, sql, false);
    if(!con.IsInvalid()) {
	    con->busy.Unlock();
    }
	return Result;
}

bool Database::WaitExecuteNA(const char* QueryString) throw()
{
	CAutoPointer<DatabaseConnection> con(GetFreeConnection());
	bool Result = SendQuery(con, QueryString, false);
    if(!con.IsInvalid()) {
	    con->busy.Unlock();
    }
	return Result;
}

bool Database::Run()
{
	if((int32_t)atomic_inc(&m_nCurThdCount) > GetMaxThdSize()) {
		atomic_dec(&m_nCurThdCount);
		return false;
	}

	CAutoPointer<DatabaseConnection> con(GetFreeConnection());

	do
	{
		int nExeSize = m_queExecute.Size();
		for(int i = 0; i < nExeSize; ++i) {
			QueryStrBuffer* pQSB = m_queExecute.ReadLock();
			if(NULL == pQSB) {
				break;
			}
			QueryStrBuffer qSB(*pQSB);
			pQSB->Clear();
			m_queExecute.ReadUnlock();
			// Execute the sql
			SendQuery(con, qSB.GetStr(), false);
		}

		int nQuerySize = m_queQuery.Size();
		for(int j = 0; j < nQuerySize; ++j) {
			CAutoPointer<AsyncQueryBase>* pPAQ = m_queQuery.ReadLock();
			if(NULL == pPAQ) {
				break;
			}
			CAutoPointer<AsyncQueryBase> pAq(*pPAQ);
			pPAQ->SetRawPointer(NULL);
			m_queQuery.ReadUnlock();
			// Query the sql
			PerformAsyncQuery(pAq, con);
		}
		
		if(m_queExecute.Size() < 1 && m_queQuery.Size() < 1) {
			atomic_dec(&m_nCurThdCount);
			// recheck
			if(m_queExecute.Size() > 0 || m_queQuery.Size() > 0) {
				atomic_inc(&m_nCurThdCount);
			} else {
				break;
			}
		}
	} while(true);

	if(!con.IsInvalid()) {
		con->busy.Unlock();
	}
	return false;
}

void Database::EndThreads()
{
	SetThreadState(THREADSTATE_TERMINATE);

	// wait
	for(int i = 0; m_nCurThdCount > 0; ++i) {
		cpu_relax(i);
	}
}

void Database::WaitBatchQuery(CAutoPointer<AsyncQueryCb> aqcb) throw()
{
	CAutoPointer<AsyncQueryBase> aq(aqcb);
	if(aq.IsInvalid()) {
		return;
	}

	if(aq->IsQueryEmpty()) {
		assert(false);
		return;
	}

	CAutoPointer<DatabaseConnection> conn(GetFreeConnection());

	int nQuerySize = aq->GetQuerySize();
	for(int i = 0; i < nQuerySize; ++i) {
		aq->SetQueryResult(i, FQuery(aq->GetQueryString(i), conn));
	}
	aq->ResetQueryString();

	if(!conn.IsInvalid()) {

		conn->busy.Unlock();
	}

	aq->Invoke();
}

void Database::AddAsyncQuery(CAutoPointer<AsyncQueryBase> aq)
{
	if(aq.IsInvalid()) {
		return;
	}

	CThreadState threadState = GetThreadState();

	if(THREADSTATE_TERMINATE == threadState) {
		return;
	} else if(THREADSTATE_AWAITING == threadState) {
		PerformAsyncQuery(aq, CAutoPointer<DatabaseConnection>());
		return;
	}

	CAutoPointer<AsyncQueryBase>* pPAQ = m_queQuery.WriteLock();
	*pPAQ = aq;
	m_queQuery.WriteUnlock();

	// Spawn Database thread
	TriggerRun();
}

void Database::TriggerRun() {
	if(m_nCurThdCount < GetMaxThdSize()) {
		ThreadPool.ExecuteTask(this);
	}
}

} // end namespace db


