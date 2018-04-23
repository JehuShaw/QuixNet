/*
 * File:   PlayerCallBack.h
 * Author: Jehu Shaw
 *
 */

#ifndef _PLAYERCALLBACK_H
#define _PLAYERCALLBACK_H

#include <assert.h>
#include "CallBack.h"
#include "PlayerBase.h"
#include "ChannelManager.h"
//////////////////////////////////////////////////////////////////////////
// One Player Callback
// PP0 Player Param 0
class GCallBackPP0 : public util::CallbackBase
{
	typedef void(*Method)(util::CWeakPointer<CPlayerBase>);
	Method m_cb;
    uint64_t m_userId;
public:
	GCallBackPP0(Method fn, uint64_t userId) 
        : m_cb(fn), m_userId(userId) {}

	void operator()() {
        CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userId));
        if(pPlayer.IsInvalid()) {
            (m_cb)(pPlayer); 
        } else {
            CScopedPlayerMutex scopedPlayerMutex(pPlayer);
            (m_cb)(pPlayer); 
        }
    }
	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const GCallBackPP0& subRight = 
			dynamic_cast<const GCallBackPP0&>(right);
		return this->m_cb == subRight.m_cb;
	}
};

// PP1 Player Param 1
template<typename P1>
class GCallBackPP1 : public util::CallbackBase
{
	typedef void(*Method)(util::CWeakPointer<CPlayerBase>,P1&);
	Method m_cb;
	P1 m_p1;
    uint64_t m_userId;
public:
	GCallBackPP1(Method fn, uint64_t userId, const P1& p1)
        : m_cb(fn), m_userId(userId), m_p1(p1) {}

	void operator()() { 
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userId));
        if(pPlayer.IsInvalid()) {
            (m_cb)(pPlayer, m_p1); 
        } else {
            CScopedPlayerMutex scopedPlayerMutex(pPlayer);
            (m_cb)(pPlayer, m_p1); 
        }
    }

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const GCallBackPP1& subRight = 
			dynamic_cast<const GCallBackPP1&>(right);
		return this->m_cb == subRight.m_cb;
	}
};

// PP2 Player Param 2
template<typename P1, typename P2>
class GCallBackPP2 : public util::CallbackBase
{
	typedef void(*Method)(util::CWeakPointer<CPlayerBase>,P1&,P2&);
	Method m_cb;
    uint64_t m_userId;
	P1 m_p1;
	P2 m_p2;
public:
	GCallBackPP2(Method fn, uint64_t userId
        , const P1& p1, const P2& p2) 
        : m_cb(fn), m_userId(userId), m_p1(p1), m_p2(p2) {}

	void operator()() {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userId));
        if(pPlayer.IsInvalid()) {
            (m_cb)(pPlayer, m_p1, m_p2); 
        } else {
            CScopedPlayerMutex scopedPlayerMutex(pPlayer);
            (m_cb)(pPlayer, m_p1, m_p2); 
        }
    }

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const GCallBackPP2& subRight = 
			dynamic_cast<const GCallBackPP2&>(right);
		return this->m_cb == subRight.m_cb;
	}
};

// PP3 Player Param 3
template<typename P1, typename P2, typename P3>
class GCallBackPP3 : public util::CallbackBase
{
	typedef void(*Method)(util::CWeakPointer<CPlayerBase>,P1&,P2&,P3&);
	Method m_cb;
    uint64_t m_userId;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
public:
	GCallBackPP3(Method fn, uint64_t userId
        , const P1& p1, const P2& p2, const P3& p3) 
        : m_cb(fn), m_userId(userId), m_p1(p1), m_p2(p2), m_p3(p3) {}

	void operator()() {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userId));
        if(pPlayer.IsInvalid()) {
            (m_cb)(pPlayer, m_p1, m_p2, m_p3); 
        } else {
            CScopedPlayerMutex scopedPlayerMutex(pPlayer);
            (m_cb)(pPlayer, m_p1, m_p2, m_p3); 
        }
    }

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const GCallBackPP3& subRight = 
			dynamic_cast<const GCallBackPP3&>(right);
		return this->m_cb == subRight.m_cb;
	}
};

// PP4 Player Param 4
template<typename P1, typename P2, typename P3, typename P4>
class GCallBackPP4 : public util::CallbackBase
{
	typedef void(*Method)(util::CWeakPointer<CPlayerBase>,P1&,P2&,P3&,P4&);
	Method m_cb;
    uint64_t m_userId;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
public:
	GCallBackPP4(Method fn, uint64_t userId
        , const P1& p1, const P2& p2, const P3& p3, const P4& p4) 
        : m_cb(fn)
        , m_userId(userId)
        , m_p1(p1)
        , m_p2(p2)
        , m_p3(p3)
        , m_p4(p4) {}

	void operator()() {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userId));
        if(pPlayer.IsInvalid()) {
            (m_cb)(pPlayer, m_p1, m_p2, m_p3, m_p4); 
        } else {
            CScopedPlayerMutex scopedPlayerMutex(pPlayer);
            (m_cb)(pPlayer, m_p1, m_p2, m_p3, m_p4); 
        }
    }

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const GCallBackPP4& subRight = 
			dynamic_cast<const GCallBackPP4&>(right);
		return this->m_cb == subRight.m_cb;
	}
};

template < class T >
class MCallbackPP0 : public util::CallbackBase
{
public:

	typedef void (T::*Method)(util::CWeakPointer<CPlayerBase>);
	MCallbackPP0(util::CWeakPointer<T> obj, Method func, uint64_t userId)
		: m_obj(obj)
		, m_func(func)
        , m_userId(userId)
	{
	}
	void operator()() { 
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userId));
        if(pPlayer.IsInvalid()) {
			util::CAutoPointer<T> pObject(m_obj.GetStrong());
			T* ptr = pObject.operator->();
			if(NULL == ptr) {
				assert(NULL != ptr);
				return;
			}
            (ptr->*m_func)(pPlayer); 
        } else {
			util::CAutoPointer<T> pObject(m_obj.GetStrong());
			T* ptr = pObject.operator->();
			if(NULL == ptr) {
				assert(NULL != ptr);
				return;
			}
            CScopedPlayerMutex scopedPlayerMutex(pPlayer);
            (ptr->*m_func)(pPlayer); 
        }
    }

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const MCallbackPP0& subRight = 
			dynamic_cast<const MCallbackPP0&>(right);
		return this->m_func == subRight.m_func;
	}
private:

	util::CWeakPointer<T> m_obj;
	Method m_func;
    uint64_t m_userId;
};

template < class T, typename P1 >
class MCallbackPP1 : public util::CallbackBase
{
public:

	typedef void (T::*Method)(util::CWeakPointer<CPlayerBase>, P1&);
	MCallbackPP1(util::CWeakPointer<T> obj, Method func, uint64_t userId, const P1& p1)
		: m_obj(obj)
		, m_func(func)
        , m_userId(userId)
		, m_p1(p1)
	{
	}

	void operator()() {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userId));
        if(pPlayer.IsInvalid()) {
			util::CAutoPointer<T> pObject(m_obj.GetStrong());
			T* ptr = pObject.operator->();
			if(NULL == ptr) {
				assert(NULL != ptr);
				return;
			}
            (ptr->*m_func)(pPlayer, m_p1); 
        } else {
			util::CAutoPointer<T> pObject(m_obj.GetStrong());
			T* ptr = pObject.operator->();
			if(NULL == ptr) {
				assert(NULL != ptr);
				return;
			}
            CScopedPlayerMutex scopedPlayerMutex(pPlayer);
            (ptr->*m_func)(pPlayer, m_p1); 
        }
    }

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const MCallbackPP1& subRight = 
			dynamic_cast<const MCallbackPP1&>(right);
		return this->m_func == subRight.m_func;
	}
private:

	util::CWeakPointer<T> m_obj;
	Method m_func;
    uint64_t m_userId;
	P1 m_p1;
};

template < class T, typename P1, typename P2 >
class MCallbackPP2 : public util::CallbackBase
{
public:

	typedef void (T::*Method)(util::CWeakPointer<CPlayerBase>, P1&, P2&);
	MCallbackPP2(util::CWeakPointer<T> obj, Method func, uint64_t userId, const P1& p1, const P2& p2)
		: m_obj(obj)
		, m_func(func)
        , m_userId(userId)
		, m_p1(p1)
		, m_p2(p2)
	{
	}

	void operator()() {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userId));
        if(pPlayer.IsInvalid()) {
			util::CAutoPointer<T> pObject(m_obj.GetStrong());
			T* ptr = pObject.operator->();
			if(NULL == ptr) {
				assert(NULL != ptr);
				return;
			}
            (ptr->*m_func)(pPlayer, m_p1, m_p2); 
        } else {
			util::CAutoPointer<T> pObject(m_obj.GetStrong());
			T* ptr = pObject.operator->();
			if(NULL == ptr) {
				assert(NULL != ptr);
				return;
			}
            CScopedPlayerMutex scopedPlayerMutex(pPlayer);
            (ptr->*m_func)(pPlayer, m_p1, m_p2); 
        }
    }

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const MCallbackPP2& subRight = 
			dynamic_cast<const MCallbackPP2&>(right);
		return this->m_func == subRight.m_func;
	}
private:

	util::CWeakPointer<T> m_obj;
	Method m_func;
    uint64_t m_userId;
	P1 m_p1;
	P2 m_p2;
};

template < class T, typename P1, typename P2, typename P3 >
class MCallbackPP3 : public util::CallbackBase
{
public:

	typedef void (T::*Method)(util::CWeakPointer<CPlayerBase>, P1&, P2&, P3&);
	MCallbackPP3(util::CWeakPointer<T> obj, Method func, uint64_t userId
        , const P1& p1, const P2& p2, const P3& p3)
		: m_obj(obj)
		, m_func(func)
        , m_userId(userId)
		, m_p1(p1)
		, m_p2(p2)
		, m_p3(p3)
	{
	}

	void operator()() {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userId));
        if(pPlayer.IsInvalid()) {
			util::CAutoPointer<T> pObject(m_obj.GetStrong());
			T* ptr = pObject.operator->();
			if(NULL == ptr) {
				assert(NULL != ptr);
				return;
			}
            (ptr->*m_func)(pPlayer, m_p1, m_p2, m_p3); 
        } else {
			util::CAutoPointer<T> pObject(m_obj.GetStrong());
			T* ptr = pObject.operator->();
			if(NULL == ptr) {
				assert(NULL != ptr);
				return;
			}
            CScopedPlayerMutex scopedPlayerMutex(pPlayer);
            (ptr->*m_func)(pPlayer, m_p1, m_p2, m_p3); 
        }
    }

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const MCallbackPP3& subRight = 
			dynamic_cast<const MCallbackPP3&>(right);
		return this->m_func == subRight.m_func;
	}
private:

	util::CWeakPointer<T> m_obj;
	Method m_func;
    uint64_t m_userId;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
};

template < class T, typename P1, typename P2, typename P3, typename P4 >
class MCallbackPP4 : public util::CallbackBase
{
public:

	typedef void (T::*Method)(util::CWeakPointer<CPlayerBase>, P1&, P2&, P3&, P4&);
	MCallbackPP4(util::CWeakPointer<T> obj, Method func, uint64_t userId
        , const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		: m_obj(obj)
		, m_func(func)
        , m_userId(userId)
		, m_p1(p1)
		, m_p2(p2)
		, m_p3(p3)
		, m_p4(p4)
	{
	}

	void operator()() {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
        util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userId));
        if(pPlayer.IsInvalid()) {
			util::CAutoPointer<T> pObject(m_obj.GetStrong());
			T* ptr = pObject.operator->();
			if(NULL == ptr) {
				assert(NULL != ptr);
				return;
			}
            (ptr->*m_func)(pPlayer, m_p1, m_p2, m_p3, m_p4); 
        } else {
			util::CAutoPointer<T> pObject(m_obj.GetStrong());
			T* ptr = pObject.operator->();
			if(NULL == ptr) {
				assert(NULL != ptr);
				return;
			}
            CScopedPlayerMutex scopedPlayerMutex(pPlayer);
            (ptr->*m_func)(pPlayer, m_p1, m_p2, m_p3, m_p4); 
        }
    }

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const MCallbackPP4& subRight = 
			dynamic_cast<const MCallbackPP4&>(right);
		return this->m_func == subRight.m_func;
	}
private:

	util::CWeakPointer<T> m_obj;
	Method m_func;
    uint64_t m_userId;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
};

//////////////////////////////////////////////////////////////////////////
// Players Callback
// PsP0 Players Param 0
class GCallBackPsP0 : public util::CallbackBase
{
	typedef void(*Method)(PLAYER_BASE_SET_T&);
	Method m_cb;
	PLAYER_ID_SET_T m_userIds;
public:
	GCallBackPsP0(Method fn, const PLAYER_ID_SET_T& userIds) 
		: m_cb(fn), m_userIds(userIds) {}

	void operator()() {
		PLAYER_BASE_SET_T players;
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		int nSize = (int)m_userIds.size();
		for(int i = 0; i < nSize; ++i) {
			util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userIds[i]));
			if(!pPlayer.IsInvalid()) {
				players.push_back(pPlayer);
			}
		}

		CScopedPlayersMutex scopedPlayersMutex(players);
		(m_cb)(players); 
	}

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const GCallBackPsP0& subRight = 
			dynamic_cast<const GCallBackPsP0&>(right);
		return this->m_cb == subRight.m_cb;
	}
};

// PsP1 Players Param 1
template<typename P1>
class GCallBackPsP1 : public util::CallbackBase
{
	typedef void(*Method)(PLAYER_BASE_SET_T&,P1&);
	Method m_cb;
	P1 m_p1;
	PLAYER_ID_SET_T m_userIds;
public:
	GCallBackPsP1(Method fn, const PLAYER_ID_SET_T& userIds, const P1& p1)
		: m_cb(fn), m_userIds(userIds), m_p1(p1) {}

	void operator()() { 
		PLAYER_BASE_SET_T players;
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		int nSize = (int)m_userIds.size();
		for(int i = 0; i < nSize; ++i) {
			util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userIds[i]));
			if(!pPlayer.IsInvalid()) {
				players.push_back(pPlayer);
			}
		}

		CScopedPlayersMutex scopedPlayersMutex(players);
		(m_cb)(players, m_p1); 
	}

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const GCallBackPsP1& subRight = 
			dynamic_cast<const GCallBackPsP1&>(right);
		return this->m_cb == subRight.m_cb;
	}
};

// PsP2 Players Param 2
template<typename P1, typename P2>
class GCallBackPsP2 : public util::CallbackBase
{
	typedef void(*Method)(PLAYER_BASE_SET_T&,P1&,P2&);
	Method m_cb;
	PLAYER_ID_SET_T m_userIds;
	P1 m_p1;
	P2 m_p2;
public:
	GCallBackPsP2(Method fn, const PLAYER_ID_SET_T& userIds
		, const P1& p1, const P2& p2) 
		: m_cb(fn), m_userIds(userIds), m_p1(p1), m_p2(p2) {}

	void operator()() {
		PLAYER_BASE_SET_T players;
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		int nSize = (int)m_userIds.size();
		for(int i = 0; i < nSize; ++i) {
			util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userIds[i]));
			if(!pPlayer.IsInvalid()) {
				players.push_back(pPlayer);
			}
		}

		CScopedPlayersMutex scopedPlayersMutex(players);
		(m_cb)(players, m_p1, m_p2); 
	}

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const GCallBackPsP2& subRight = 
			dynamic_cast<const GCallBackPsP2&>(right);
		return this->m_cb == subRight.m_cb;
	}
};

// PsP3 Players Param 3
template<typename P1, typename P2, typename P3>
class GCallBackPsP3 : public util::CallbackBase
{
	typedef void(*Method)(PLAYER_BASE_SET_T&,P1&,P2&,P3&);
	Method m_cb;
	PLAYER_ID_SET_T m_userIds;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
public:
	GCallBackPsP3(Method fn, const PLAYER_ID_SET_T& userIds
		, const P1& p1, const P2& p2, const P3& p3) 
		: m_cb(fn), m_userIds(userIds), m_p1(p1), m_p2(p2), m_p3(p3) {}

	void operator()() {
		PLAYER_BASE_SET_T players;
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		int nSize = (int)m_userIds.size();
		for(int i = 0; i < nSize; ++i) {
			util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userIds[i]));
			if(!pPlayer.IsInvalid()) {
				players.push_back(pPlayer);
			}
		}

		CScopedPlayersMutex scopedPlayersMutex(players);
		(m_cb)(players, m_p1, m_p2, m_p3); 
	}

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const GCallBackPsP3& subRight = 
			dynamic_cast<const GCallBackPsP3&>(right);
		return this->m_cb == subRight.m_cb;
	}
};

// PsP4 Players Param 4
template<typename P1, typename P2, typename P3, typename P4>
class GCallBackPsP4 : public util::CallbackBase
{
	typedef void(*Method)(PLAYER_BASE_SET_T&,P1&,P2&,P3&,P4&);
	Method m_cb;
	PLAYER_ID_SET_T m_userIds;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
public:
	GCallBackPsP4(Method fn, const PLAYER_ID_SET_T& userIds
		, const P1& p1, const P2& p2, const P3& p3, const P4& p4) 
		: m_cb(fn)
		, m_userIds(userIds)
		, m_p1(p1)
		, m_p2(p2)
		, m_p3(p3)
		, m_p4(p4) {}

	void operator()() {
		PLAYER_BASE_SET_T players;
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		int nSize = (int)m_userIds.size();
		for(int i = 0; i < nSize; ++i) {
			util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userIds[i]));
			if(!pPlayer.IsInvalid()) {
				players.push_back(pPlayer);
			}
		}

		CScopedPlayersMutex scopedPlayersMutex(players);
		(m_cb)(players, m_p1, m_p2, m_p3, m_p4); 
	}

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const GCallBackPsP4& subRight = 
			dynamic_cast<const GCallBackPsP4&>(right);
		return this->m_cb == subRight.m_cb;
	}
};

template < class T >
class MCallbackPsP0 : public util::CallbackBase
{
public:

	typedef void (T::*Method)(PLAYER_BASE_SET_T&);
	MCallbackPsP0(util::CWeakPointer<T> obj, Method func, const PLAYER_ID_SET_T& userIds)
		: m_obj(obj)
		, m_func(func)
		, m_userIds(userIds)
	{
	}
	void operator()() { 
		PLAYER_BASE_SET_T players;
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		int nSize = (int)m_userIds.size();
		for(int i = 0; i < nSize; ++i) {
			util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userIds[i]));
			if(!pPlayer.IsInvalid()) {
				players.push_back(pPlayer);
			}
		}
		util::CAutoPointer<T> pObject(m_obj.GetStrong());
		T* ptr = pObject.operator->();
		if(NULL == ptr) {
			assert(NULL != ptr);
			return;
		}
		CScopedPlayersMutex scopedPlayersMutex(players);
		(ptr->*m_func)(players); 
	}

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const MCallbackPsP0& subRight = 
			dynamic_cast<const MCallbackPsP0&>(right);
		return this->m_func == subRight.m_func;
	}
private:

	util::CWeakPointer<T> m_obj;
	Method m_func;
	PLAYER_ID_SET_T m_userIds;
};

template < class T, typename P1 >
class MCallbackPsP1 : public util::CallbackBase
{
public:

	typedef void (T::*Method)(PLAYER_BASE_SET_T&, P1&);
	MCallbackPsP1(util::CWeakPointer<T> obj, Method func, const PLAYER_ID_SET_T& userIds, const P1& p1)
		: m_obj(obj)
		, m_func(func)
		, m_userIds(userIds)
		, m_p1(p1)
	{
	}

	void operator()() {
		PLAYER_BASE_SET_T players;
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		int nSize = (int)m_userIds.size();
		for(int i = 0; i < nSize; ++i) {
			util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userIds[i]));
			if(!pPlayer.IsInvalid()) {
				players.push_back(pPlayer);
			}
		}
		util::CAutoPointer<T> pObject(m_obj.GetStrong());
		T* ptr = pObject.operator->();
		if(NULL == ptr) {
			assert(NULL != ptr);
			return;
		}
		CScopedPlayersMutex scopedPlayersMutex(players);
		(ptr->*m_func)(players, m_p1); 
	}

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const MCallbackPsP1& subRight = 
			dynamic_cast<const MCallbackPsP1&>(right);
		return this->m_func == subRight.m_func;
	}
private:

	util::CWeakPointer<T> m_obj;
	Method m_func;
	PLAYER_ID_SET_T m_userIds;
	P1 m_p1;
};

template < class T, typename P1, typename P2 >
class MCallbackPsP2 : public util::CallbackBase
{
public:

	typedef void (T::*Method)(PLAYER_BASE_SET_T&, P1&, P2&);
	MCallbackPsP2(util::CWeakPointer<T> obj, Method func, const PLAYER_ID_SET_T& userIds, const P1& p1, const P2& p2)
		: m_obj(obj)
		, m_func(func)
		, m_userIds(userIds)
		, m_p1(p1)
		, m_p2(p2)
	{
	}

	void operator()() {
		PLAYER_BASE_SET_T players;
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		int nSize = (int)m_userIds.size();
		for(int i = 0; i < nSize; ++i) {
			util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userIds[i]));
			if(!pPlayer.IsInvalid()) {
				players.push_back(pPlayer);
			}
		}
		util::CAutoPointer<T> pObject(m_obj.GetStrong());
		T* ptr = pObject.operator->();
		if(NULL == ptr) {
			assert(NULL != ptr);
			return;
		}
		CScopedPlayersMutex scopedPlayersMutex(players);
		(ptr->*m_func)(players, m_p1, m_p2); 
	}

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const MCallbackPsP2& subRight = 
			dynamic_cast<const MCallbackPsP2&>(right);
		return this->m_func == subRight.m_func;
	}
private:

	util::CWeakPointer<T> m_obj;
	Method m_func;
	PLAYER_ID_SET_T m_userIds;
	P1 m_p1;
	P2 m_p2;
};

template < class T, typename P1, typename P2, typename P3 >
class MCallbackPsP3 : public util::CallbackBase
{
public:

	typedef void (T::*Method)(PLAYER_BASE_SET_T&, P1&, P2&, P3&);
	MCallbackPsP3(util::CWeakPointer<T> obj, Method func, const PLAYER_ID_SET_T& userIds
		, const P1& p1, const P2& p2, const P3& p3)
		: m_obj(obj)
		, m_func(func)
		, m_userIds(userIds)
		, m_p1(p1)
		, m_p2(p2)
		, m_p3(p3)
	{
	}

	void operator()() {
		PLAYER_BASE_SET_T players;
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		int nSize = (int)m_userIds.size();
		for(int i = 0; i < nSize; ++i) {
			util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userIds[i]));
			if(!pPlayer.IsInvalid()) {
				players.push_back(pPlayer);
			}
		}
		util::CAutoPointer<T> pObject(m_obj.GetStrong());
		T* ptr = pObject.operator->();
		if(NULL == ptr) {
			assert(NULL != ptr);
			return;
		}
		CScopedPlayersMutex scopedPlayersMutex(players);
		(ptr->*m_func)(players, m_p1, m_p2, m_p3); 
	}

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const MCallbackPsP3& subRight = 
			dynamic_cast<const MCallbackPsP3&>(right);
		return this->m_func == subRight.m_func;
	}
private:

	util::CWeakPointer<T> m_obj;
	Method m_func;
	PLAYER_ID_SET_T m_userIds;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
};

template < class T, typename P1, typename P2, typename P3, typename P4 >
class MCallbackPsP4 : public util::CallbackBase
{
public:

	typedef void (T::*Method)(PLAYER_BASE_SET_T&, P1&, P2&, P3&, P4&);
	MCallbackPsP4(util::CWeakPointer<T> obj, Method func, const PLAYER_ID_SET_T& userIds
		, const P1& p1, const P2& p2, const P3& p3, const P4& p4)
		: m_obj(obj)
		, m_func(func)
		, m_userIds(userIds)
		, m_p1(p1)
		, m_p2(p2)
		, m_p3(p3)
		, m_p4(p4)
	{
	}

	void operator()() {
		PLAYER_BASE_SET_T players;
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		int nSize = (int)m_userIds.size();
		for(int i = 0; i < nSize; ++i) {
			util::CAutoPointer<CPlayerBase> pPlayer(pChlMgr->GetPlayer(m_userIds[i]));
			if(!pPlayer.IsInvalid()) {
				players.push_back(pPlayer);
			}
		}
		util::CAutoPointer<T> pObject(m_obj.GetStrong());
		T* ptr = pObject.operator->();
		if(NULL == ptr) {
			assert(NULL != ptr);
			return;
		}
		CScopedPlayersMutex scopedPlayersMutex(players);
		(ptr->*m_func)(players, m_p1, m_p2, m_p3, m_p4); 
	}

	void Invoke() { operator()(); }

	bool Equal(const util::CallbackBase& right)const {
		const MCallbackPsP4& subRight = 
			dynamic_cast<const MCallbackPsP4&>(right);
		return this->m_func == subRight.m_func;
	}
private:

	util::CWeakPointer<T> m_obj;
	Method m_func;
	PLAYER_ID_SET_T m_userIds;
	P1 m_p1;
	P2 m_p2;
	P3 m_p3;
	P4 m_p4;
};

#endif /* _PLAYERCALLBACK_H */
