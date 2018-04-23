/* 
 * File:   WorkerModule.h
 * Author: Jehu Shaw
 *
 * Created on 2014_4_4 11:25
 */

#pragma once

#include "TPriorityQueue.h"
#include "worker.rpcz.h"
#include "IChannelControl.h"
#include "IChannelValue.h"
#include "dbstl_map.h"

class Db;
class DbEnv;

class CDBChannel {
public:
	CDBChannel()
		: m_nUserCount(0)
		, m_nServCount(0)
	{
#if COMPILER == COMPILER_MICROSOFT
		BUILD_BUG_ON(sizeof(uint32_t) < sizeof(long));
#endif
	}
	uint32_t GetUserCount() const {
		return m_nUserCount;
	}
	uint32_t GetServCount() const {
		return m_nServCount;
	}
	void SetUserCount(uint32_t nCount) {
		atomic_xchg(&m_nUserCount, nCount);
	}
	void SetServCount(uint32_t nCount) {
		atomic_xchg(&m_nServCount, nCount);
	}
	void AtomicDecUserCount() throw() {
		uint32_t count;
		do {
			count = (uint32_t)m_nUserCount;
			if(count == 0) {
				return;
			}
		} while (atomic_cmpxchg(&m_nUserCount, (count < 0 ?
			(count + 1) : (count - 1)), count) != count);
	}
	void AtomicDecServCount() throw() {
		uint32_t count;
		do {
			count = (uint32_t)m_nServCount;
			if(count == 0) {
				return;
			}
		} while (atomic_cmpxchg(&m_nServCount, (count < 0 ?
			(count + 1) : (count - 1)), count) != count);
	}
private:
	volatile uint32_t m_nUserCount;
	volatile uint32_t m_nServCount;
};

class CWorkerModule : public IChannelControl
{
public:
	CWorkerModule(DbEnv* pDbEnv,
		uint16_t thisServId,
		const std::string& moduleName);

	CWorkerModule(DbEnv* pDbEnv,
		uint16_t thisServId,
		const std::string& moduleName,
		const std::string& endPoint,
		uint16_t serverId);

	CWorkerModule(DbEnv* pDbEnv,
		uint16_t thisServId,
		const std::string& moduleName,
		const std::string& endPoint,
		uint16_t serverId,
		const std::string& acceptAddress,
		const std::string& processPath,
		const std::string& projectName,
		uint16_t serverRegion);

	~CWorkerModule(void);

	virtual void OnRegister();

	virtual void OnRemove();

	virtual std::vector<int> ListNotificationInterests();

	virtual InterestList ListProtocolInterests();

	virtual util::CAutoPointer<mdl::IObserverRestricted> FullProtocolInterests();

	virtual void HandleNotification(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	virtual bool CreatChannel(uint16_t serverId, const std::string& endPoint, uint16_t serverType);

	virtual bool CreatChannel(uint16_t serverId, const std::string& endPoint, uint16_t serverType,
		const std::string& acceptAddress, const std::string& processPath, const std::string& projectName,
		uint16_t serverRegion);

	virtual bool RemoveChannel(uint16_t serverId);

	virtual int ChannelCount() const;

	virtual void IterateChannel(std::vector<util::CAutoPointer<IChannelValue> >& outChannels) const;

	virtual util::CAutoPointer<IChannelValue> GetLowLoadUserChnl() const;

	virtual util::CAutoPointer<IChannelValue> GetChnlByDirServId(uint16_t serverId) const;

protected:
	void HandleMessage(const util::CWeakPointer<mdl::INotification>& request,
		util::CWeakPointer<mdl::IResponse>& reply);

	void HandleDefault(const util::CWeakPointer<::node::DataPacket>& pDpRequest,
		util::CWeakPointer<::node::DataPacket>& pDpResponse);

	void HandleLogout(const util::CWeakPointer<::node::DataPacket>& pDpRequest,
		util::CWeakPointer<::node::DataPacket>& pDpResponse);

	bool GetChnlByBalUserId(uint64_t userId,
		util::CAutoPointer<IChannelValue>& outChannel) const;
	
	void UpdateChnlByBalUserId(uint64_t userId,
		util::CAutoPointer<IChannelValue>& channel);

	void RemoveBalUserId(uint64_t userId);

	bool GetChnlByBalServId(uint16_t serverId,
		util::CAutoPointer<IChannelValue>& outChannel) const;

	void UpdateChnlByBalServId(uint16_t serverId,
		util::CAutoPointer<IChannelValue>& channel);

	void RemoveBalServId(uint16_t serverId);

	void RemoveInvalidServId();

protected:
	util::CAutoPointer<std::vector<int> > m_notifications;
	thd::CCriticalSection m_notifMutex;
	util::CAutoPointer<std::vector<int> > m_protocols;
	thd::CCriticalSection m_protoMutex;

	util::CTPriorityQueue m_lessUserQueue;
	typedef dbstl::db_map<uint64_t, uint16_t, dbstl::ElementHolder<uint16_t> > BALUSERID_TO_CHNL_T;
	BALUSERID_TO_CHNL_T m_balUserIdChnls;
	thd::CSpinRWLock m_rwBalUser;

	util::CTPriorityQueue m_lessServQueue;
	typedef dbstl::db_map<uint16_t, uint16_t, dbstl::ElementHolder<uint16_t> > BALSERVID_TO_CHNL_T;
	BALSERVID_TO_CHNL_T m_balServIdChnls;
	thd::CSpinRWLock m_rwBalServer;

	typedef std::map<uint16_t, util::CAutoPointer<IChannelValue> > DIRSERVID_TO_CHNL_T;
	DIRSERVID_TO_CHNL_T m_dirServIdChnls;
	thd::CSpinRWLock m_rwDirServer;

	typedef dbstl::db_map<uint16_t, CDBChannel> DBSERVID_TO_CHNL_T;
	DBSERVID_TO_CHNL_T m_dbServIdChnls;
	thd::CSpinRWLock m_rwDbChnls;

	util::CAutoPointer<CWorkerModule> m_pThis;
	Db* m_pDb;
	thd::CSpinLock m_lkDB;
};

