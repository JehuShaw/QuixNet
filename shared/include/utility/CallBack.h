/*
 * File:   CallBack.h
 * Author: Jehu Shaw
 *
 */

#ifndef CALLBACK_H
#define CALLBACK_H

#include "WeakPointer.h"

namespace util {

class CallbackBase {
public:
	virtual ~CallbackBase() {}
	virtual void Invoke() = 0;
	virtual bool Equal(const CallbackBase& right)const = 0;
};

class CallbackFP {
public:
	typedef void(*Method)();

	CallbackFP(Method cb) : m_cb(cb) {}
	void operator()() { m_cb();}
	void Invoke() { m_cb(); }
	void Set(Method cb) { m_cb = cb; }

private:
	Method m_cb;
};

class CallBackFuncP0 : public CallbackBase
{
public:
	typedef void(*Method)();

	CallBackFuncP0(Method fn) : m_cb(fn) {}
	void operator()() { (m_cb)(); }

	void Invoke() { operator()(); }

	bool Equal(const CallbackBase& right)const {
		const CallBackFuncP0& subRight =
			dynamic_cast<const CallBackFuncP0&>(right);
		return m_cb == subRight.m_cb;
	}

private:
	Method m_cb;
};

template<typename P1>
class CallBackFuncP1 : public CallbackBase
{
public:
	typedef void(*Method)(P1&);

	CallBackFuncP1(Method fn, P1 p1)
		: m_cb(fn), m_p1(p1){}

	void operator()() { (m_cb)(m_p1); }

	void Invoke() { operator()(); }

	bool Equal(const CallbackBase& right)const {
		const CallBackFuncP1& subRight =
			dynamic_cast<const CallBackFuncP1&>(right);
		return m_cb == subRight.m_cb;
	}

private:
	Method m_cb;
	P1 m_p1;
};

template<typename P1, typename P2>
class CallBackFuncP2 : public CallbackBase
{
public:
	typedef void(*Method)(P1&, P2&);

	CallBackFuncP2(Method fn, P1 p1, P2 p2)
		: m_cb(fn), m_p1(p1), m_p2(p2) {}
	void operator()() { (m_cb)(m_p1, m_p2); }
	void Invoke() { operator()(); }

	bool Equal(const CallbackBase& right)const {
		const CallBackFuncP2& subRight =
			dynamic_cast<const CallBackFuncP2&>(right);
		return m_cb == subRight.m_cb;
	}

private:
	Method m_cb;
	P1 m_p1;
	P2 m_p2;
};

template<typename P1, typename P2, typename P3>
class CallBackFuncP3 : public CallbackBase
{
public:
	typedef void(*Method)(P1&, P2&, P3&);

	CallBackFuncP3(Method fn, P1 p1, P2 p2, P3 p3)
		: m_cb(fn), m_p1(p1), m_p2(p2), m_p3(p3) {}
	void operator()() { (m_cb)(m_p1,m_p2,m_p3); }
	void Invoke() { operator()(); }

	bool Equal(const CallbackBase& right)const {
		const CallBackFuncP3& subRight =
			dynamic_cast<const CallBackFuncP3&>(right);
		return m_cb == subRight.m_cb;
	}

private:
	Method m_cb;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
};

template<typename P1, typename P2, typename P3, typename P4>
class CallBackFuncP4 : public CallbackBase
{
public:
	typedef void(*Method)(P1&, P2&, P3&, P4&);

	CallBackFuncP4(Method fn, P1 p1, P2 p2, P3 p3, P4 p4)
		: m_cb(fn), m_p1(p1), m_p2(p2), m_p3(p3), m_p4(p4) {}
	void operator()() { (m_cb)(m_p1, m_p2, m_p3, m_p4); }
	void Invoke() { operator()(); }

	bool Equal(const CallbackBase& right)const {
		const CallBackFuncP4& subRight =
			dynamic_cast<const CallBackFuncP4&>(right);
		return m_cb == subRight.m_cb;
	}

private:
	Method m_cb;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
};

template < class T >
class CallbackMFnP0 : public CallbackBase
{
public:
	typedef void (T::*Method)();

	CallbackMFnP0(util::CWeakPointer<T> obj, Method func)
		: m_obj(obj)
		, m_func(func)
	{
	}
	void operator()() {
		util::CAutoPointer<T> pObject(m_obj.GetStrong());
		if(pObject.IsInvalid()) {
			assert(false);
			return;
		}
		T* ptr = pObject.operator->();
		(ptr->*m_func)();
	}

	void Invoke() { operator()(); }

	bool Equal(const CallbackBase& right)const {
		const CallbackMFnP0& subRight =
			dynamic_cast<const CallbackMFnP0&>(right);
		return m_obj == subRight.m_obj
			&& m_func == subRight.m_func;
	}

private:
	util::CWeakPointer<T> m_obj;
	Method m_func;
};

template < class T, typename P1 >
class CallbackMFnP1 : public CallbackBase
{
public:
	typedef void (T::*Method)(P1&);

	CallbackMFnP1(util::CWeakPointer<T> obj, Method func, P1 p1)
		: m_obj(obj)
		, m_func(func)
		, m_p1(p1)
	{
	}

	void operator()() {
		util::CAutoPointer<T> pObject(m_obj.GetStrong());
		if(pObject.IsInvalid()) {
			assert(false);
			return;
		}
		T* ptr = pObject.operator->();
		(ptr->*m_func)(m_p1);
	}

	void Invoke() { operator()(); }

	bool Equal(const CallbackBase& right)const {
		const CallbackMFnP1& subRight =
			dynamic_cast<const CallbackMFnP1&>(right);
		return m_obj == subRight.m_obj
			&& m_func == subRight.m_func;
	}

private:
	util::CWeakPointer<T> m_obj;
	Method m_func;
	P1 m_p1;
};

template < class T, typename P1, typename P2 >
class CallbackMFnP2 : public CallbackBase
{
public:
	typedef void (T::*Method)(P1&, P2&);

	CallbackMFnP2(util::CWeakPointer<T> obj, Method func, P1 p1, P2 p2)
		: m_obj(obj)
		, m_func(func)
		, m_p1(p1)
		, m_p2(p2)
	{
	}

	void operator()() {
		util::CAutoPointer<T> pObject(m_obj.GetStrong());
		if(pObject.IsInvalid()) {
			assert(false);
			return;
		}
		T* ptr = pObject.operator->();
		(ptr->*m_func)(m_p1, m_p2);
	}

	void Invoke() { operator()(); }

	bool Equal(const CallbackBase& right)const {
		const CallbackMFnP2& subRight =
			dynamic_cast<const CallbackMFnP2&>(right);
		return m_obj == subRight.m_obj
			&& m_func == subRight.m_func;
	}

private:
	util::CWeakPointer<T> m_obj;
	Method m_func;
	P1 m_p1;
	P2 m_p2;
};

template < class T, typename P1, typename P2, typename P3 >
class CallbackMFnP3 : public CallbackBase
{
public:
	typedef void (T::*Method)(P1&, P2&, P3&);

	CallbackMFnP3(util::CWeakPointer<T> obj, Method func, P1 p1, P2 p2, P3 p3)
		: m_obj(obj)
		, m_func(func)
		, m_p1(p1)
		, m_p2(p2)
		, m_p3(p3)
	{
	}

	void operator()() {
		util::CAutoPointer<T> pObject(m_obj.GetStrong());
		if(pObject.IsInvalid()) {
			assert(false);
			return;
		}
		T* ptr = pObject.operator->();
		(ptr->*m_func)(m_p1, m_p2, m_p3);
	}

	void Invoke() { operator()(); }

	bool Equal(const CallbackBase& right)const {
		const CallbackMFnP3& subRight =
			dynamic_cast<const CallbackMFnP3&>(right);
		return m_obj == subRight.m_obj
			&& m_func == subRight.m_func;
	}

private:
	util::CWeakPointer<T> m_obj;
	Method m_func;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
};

template < class T, typename P1, typename P2, typename P3, typename P4 >
class CallbackMFnP4 : public CallbackBase
{
public:
	typedef void (T::*Method)(P1&, P2&, P3&, P4&);

	CallbackMFnP4(util::CWeakPointer<T> obj, Method func, P1 p1, P2 p2, P3 p3, P4 p4)
		: m_obj(obj)
		, m_func(func)
		, m_p1(p1)
		, m_p2(p2)
		, m_p3(p3)
		, m_p4(p4)
	{
	}

	void operator()() {
		util::CAutoPointer<T> pObject(m_obj.GetStrong());
		if(pObject.IsInvalid()) {
			assert(false);
			return;
		}
		T* ptr = pObject.operator->();
		(ptr->*m_func)(m_p1, m_p2, m_p3, m_p4);
	}

	void Invoke() { operator()(); }

	bool Equal(const CallbackBase& right)const {
		const CallbackMFnP4& subRight =
			dynamic_cast<const CallbackMFnP4&>(right);
		return m_obj == subRight.m_obj
			&& m_func == subRight.m_func;
	}

private:
	util::CWeakPointer<T> m_obj;
	Method m_func;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
};

//////////////////////////////////////////////////////////////////////////
// Callback Method Facade
FORCEINLINE CAutoPointer<CallbackBase> CBMethod(CallBackFuncP0::Method fn) {
	CAutoPointer<CallBackFuncP0>
		callback(new CallBackFuncP0(fn));
	return callback;
}

template<typename P1>
FORCEINLINE CAutoPointer<CallbackBase> CBMethod(typename CallBackFuncP1<P1>::Method fn, P1 p1) {
	CAutoPointer<CallBackFuncP1<P1> >
		callback(new CallBackFuncP1<P1>(fn, p1));
	return callback;
}

template<typename P1, typename P2>
FORCEINLINE CAutoPointer<CallbackBase> CBMethod(typename CallBackFuncP2<P1, P2>::Method fn, P1 p1, P2 p2) {
	CAutoPointer<CallBackFuncP2<P1, P2> >
		callback(new CallBackFuncP2<P1, P2>(fn, p1, p2));
	return callback;
}

template<typename P1, typename P2, typename P3>
FORCEINLINE CAutoPointer<CallbackBase> CBMethod(typename CallBackFuncP3<P1, P2, P3>::Method fn, P1 p1, P2 p2, P3 p3) {
	CAutoPointer<CallBackFuncP3<P1, P2, P3> >
		callback(new CallBackFuncP3<P1, P2, P3>(fn, p1, p2, p3));
	return callback;
}

template<typename P1, typename P2, typename P3, typename P4>
FORCEINLINE CAutoPointer<CallbackBase> CBMethod(typename CallBackFuncP4<P1, P2, P3, P4>::Method fn, P1 p1, P2 p2, P3 p3, P4 p4) {
	CAutoPointer<CallBackFuncP4<P1, P2, P3, P4> >
		callback(new CallBackFuncP4<P1, P2, P3, P4>(fn, p1, p2, p3, p4));
	return callback;
}

template <class T>
FORCEINLINE CAutoPointer<CallbackBase> CBMethod(
	CWeakPointer<T> obj,
	typename CallbackMFnP0<T>::Method fn)
{
	CAutoPointer<CallbackMFnP0<T> >
		callback(new CallbackMFnP0<T>(obj, fn));
	return callback;
}

template <class T, typename P1>
FORCEINLINE CAutoPointer<CallbackBase> CBMethod(
	CWeakPointer<T> obj,
	typename CallbackMFnP1<T, P1>::Method fn,
	P1 p1)
{
	CAutoPointer<CallbackMFnP1<T, P1> >
		callback(new CallbackMFnP1<T, P1>(obj, fn, p1));
	return callback;
}

template <class T, typename P1, typename P2>
FORCEINLINE CAutoPointer<CallbackBase> CBMethod(
	CWeakPointer<T> obj,
	typename CallbackMFnP2<T, P1, P2>::Method fn,
	P1 p1,
	P2 p2)
{
	CAutoPointer<CallbackMFnP2<T, P1, P2> >
		callback(new CallbackMFnP2<T, P1, P2>(obj, fn, p1, p2));
	return callback;
}

template <class T, typename P1, typename P2, typename P3>
FORCEINLINE CAutoPointer<CallbackBase> CBMethod(
	CWeakPointer<T> obj,
	typename CallbackMFnP3<T, P1, P2, P3>::Method fn,
	P1 p1,
	P2 p2,
	P3 p3)
{
	CAutoPointer<CallbackMFnP3<T, P1, P2, P3> >
		callback(new CallbackMFnP3<T, P1, P2, P3>(obj, fn, p1, p2, p3));
	return callback;
}

template <class T, typename P1, typename P2, typename P3, typename P4>
FORCEINLINE CAutoPointer<CallbackBase> CBMethod(
	CWeakPointer<T> obj,
	typename CallbackMFnP4<T, P1, P2, P3, P4>::Method fn,
	P1 p1,
	P2 p2,
	P3 p3,
	P4 p4)
{
	CAutoPointer<CallbackMFnP4<T, P1, P2, P3, P4> >
		callback(new CallbackMFnP4<T, P1, P2, P3, P4>(obj, fn, p1, p2, p3, p4));
	return callback;
}

}

#endif /* CALLBACK_H */
