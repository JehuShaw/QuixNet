/* 
 * File:   RankModule.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef _RANKMODULE_H
#define	_RANKMODULE_H

#include "ModuleManager.h"
#include "SpinRWLock.h"
#include "RankSetMT.h"
#include "CThreads.h"

class CRankData;

class CRankModule : public mdl::CModule, public thd::CThread  {
public:
    CRankModule(const char* name);
    virtual ~CRankModule();

	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	virtual bool Run();

private:
    void HandleRankUpdate(const util::CWeakPointer<mdl::INotification>& request,
        util::CWeakPointer<mdl::IResponse>& reply);
    void HandleRankRequest(const util::CWeakPointer<mdl::INotification>& request,
        util::CWeakPointer<mdl::IResponse>& reply);

private:
    void LoadFromDB();

    inline uint16_t GetRankCacheId()const {
        return m_nRankCacheId;
    }
private:
	uint16_t m_nRankCacheId;
	volatile bool m_bLoad;
	typedef util::CRankSetMT<uint64_t, int32_t, CRankData> RANK_SET_T;
	RANK_SET_T m_rankSet;

};

#endif	/* _RANKMODULE_H */

