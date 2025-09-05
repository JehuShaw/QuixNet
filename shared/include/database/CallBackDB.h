/*
 * File:   CallBackDB.h
 * Author: Jehu Shaw
 *
 */

#ifndef CALLBACKDB_H
#define CALLBACKDB_H

#include <vector>
#include <assert.h>
#include "WeakPointer.h"

namespace db {

class QueryResult;
struct AsyncQueryResult;
typedef std::vector<AsyncQueryResult> QueryResultVector;

class SQLCallbackBase
{
public:
	virtual ~SQLCallbackBase() {}
	virtual void Invoke(QueryResultVector & arg) = 0;
};

template<class T>
class SQLClassCallbackP0 : public SQLCallbackBase
{
	typedef void (T::*SCMethod)(QueryResultVector & arg);
	util::CWeakPointer<T> m_pObject;
	SCMethod m_pMethod;
public:
	SQLClassCallbackP0(util::CWeakPointer<T> pObject, SCMethod pMethod)
		: SQLCallbackBase(), m_pObject(pObject), m_pMethod(pMethod) {}
	~SQLClassCallbackP0() {}
	void Invoke(QueryResultVector & arg) {
		util::CAutoPointer<T> pObject(m_pObject.GetStrong());
		if(pObject.IsInvalid()) {
			assert(false);
			return;
		}
		T* ptr = pObject.operator->();
		(ptr->*m_pMethod)(arg);
	}
};

template<class T, typename P1>
class SQLClassCallbackP1 : public SQLCallbackBase
{
	typedef void (T::*SCMethod)(QueryResultVector & arg, P1 & p1);
	util::CWeakPointer<T> m_pObject;
	SCMethod m_pMethod;
	P1 m_p1;
public:
	SQLClassCallbackP1(util::CWeakPointer<T> pObject, SCMethod pMethod, const P1 & p1)
		: SQLCallbackBase(), m_pObject(pObject), m_pMethod(pMethod), m_p1(p1) {}
	~SQLClassCallbackP1() {}
	void Invoke(QueryResultVector & arg) {
		util::CAutoPointer<T> pObject(m_pObject.GetStrong());
		if(pObject.IsInvalid()) {
			assert(false);
			return;
		}
		T* ptr = pObject.operator->();
		(ptr->*m_pMethod)(arg, m_p1);
	}
};

template<class T, typename P1, typename P2>
class SQLClassCallbackP2 : public SQLCallbackBase
{
	typedef void (T::*SCMethod)(QueryResultVector & arg, P1 & p1, P2 & p2);
	util::CWeakPointer<T> m_pObject;
	SCMethod m_pMethod;
	P1 m_p1;
	P2 m_p2;
public:
	SQLClassCallbackP2(util::CWeakPointer<T> pObject, SCMethod pMethod, const P1 & p1, const P2 & p2) 
		: SQLCallbackBase(), m_pObject(pObject), m_pMethod(pMethod), m_p1(p1), m_p2(p2) {}
	~SQLClassCallbackP2() {}
	void Invoke(QueryResultVector & arg) {
		util::CAutoPointer<T> pObject(m_pObject.GetStrong());
		if(pObject.IsInvalid()) {
			assert(false);
			return;
		}
		T* ptr = pObject.operator->();
		(ptr->*m_pMethod)(arg, m_p1, m_p2);
	}
};

template<class T, typename P1, typename P2, typename P3>
class SQLClassCallbackP3 : public SQLCallbackBase
{
	typedef void (T::*SCMethod)(QueryResultVector & arg, P1 & p1, P2 & p2, P3 & p3);
	util::CWeakPointer<T> m_pObject;
	SCMethod m_pMethod;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
public:
	SQLClassCallbackP3(util::CWeakPointer<T> pObject, SCMethod pMethod, const P1 & p1, const P2 & p2, const P3 & p3) 
		: SQLCallbackBase(), m_pObject(pObject), m_pMethod(pMethod), m_p1(p1), m_p2(p2), m_p3(p3) {}
	~SQLClassCallbackP3() {}
	void Invoke(QueryResultVector & arg) {
		util::CAutoPointer<T> pObject(m_pObject.GetStrong());
		if(pObject.IsInvalid()) {
			assert(false);
			return;
		}
		T* ptr = pObject.operator->();
		(ptr->*m_pMethod)(arg, m_p1, m_p2, m_p3);
	}
};

template<class T, typename P1, typename P2, typename P3, typename P4>
class SQLClassCallbackP4 : public SQLCallbackBase
{
	typedef void (T::*SCMethod)(QueryResultVector & arg, P1 & p1, P2 & p2, P3 & p3, P4 & p4);
	util::CWeakPointer<T> m_pObject;
	SCMethod m_pMethod;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
public:
	SQLClassCallbackP4(util::CWeakPointer<T> pObject, SCMethod imethod, const P1 & p1, const P2 & p2, const P3 & p3, const P4 & p4) 
		: SQLCallbackBase(), m_pObject(pObject), m_pMethod(imethod), m_p1(p1), m_p2(p2), m_p3(p3), m_p4(p4) {}
	~SQLClassCallbackP4() {}
	void Invoke(QueryResultVector & arg) {
		util::CAutoPointer<T> pObject(m_pObject.GetStrong());
		if(pObject.IsInvalid()) {
			assert(false);
			return;
		}
		T* ptr = pObject.operator->();
		(ptr->*m_pMethod)(arg, m_p1, m_p2, m_p3, m_p4);
	}
};

class SQLFuncCallbackP0 : public SQLCallbackBase
{
	typedef void(*SCMethod)(QueryResultVector & arg);
	SCMethod m_pMethod;
public:
	SQLFuncCallbackP0(SCMethod pMethod) : SQLCallbackBase(), m_pMethod(pMethod) {}
	~SQLFuncCallbackP0() {}
	void Invoke(QueryResultVector & arg) { m_pMethod(arg); }
};

template<typename T1>
class SQLFuncCallbackP1 : public SQLCallbackBase
{
	typedef void(*SCMethod)(QueryResultVector & arg, T1 & p1);
	SCMethod m_pMethod;
	T1 m_p1;
public:
	SQLFuncCallbackP1(SCMethod pMethod, const T1 & p1) : SQLCallbackBase(), m_pMethod(pMethod), m_p1(p1) {}
	~SQLFuncCallbackP1() {}
	void Invoke(QueryResultVector & arg) { m_pMethod(arg, m_p1); }
};

template<typename T1, typename T2>
class SQLFuncCallbackP2 : public SQLCallbackBase
{
	typedef void(*SCMethod)(QueryResultVector & arg, T1 & p1, T2 & p2);
	SCMethod m_pMethod;
	T1 m_p1;
	T2 m_p2;
public:
	SQLFuncCallbackP2(SCMethod pMethod, const T1 & p1, const T2 & p2)
		: SQLCallbackBase(), m_pMethod(pMethod), m_p1(p1), m_p2(p2) {}
	~SQLFuncCallbackP2() {}
	void Invoke(QueryResultVector & arg) { m_pMethod(arg, m_p1, m_p2); }
};

}

#endif /* CALLBACKDB_H */
