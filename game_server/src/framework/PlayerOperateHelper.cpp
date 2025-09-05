#include "PlayerOperateHelper.h"
#include "SpinEvent.h"
#include "ThreadPool.h"

class CCtrlResponse : public util::PoolBase<CCtrlResponse> {
public:
	CCtrlResponse() {
		m_event.Suspend();
	}

	inline void Wait() {
		m_event.Wait();
	}

	inline bool Wait(uint32_t msec) {
		return m_event.Wait(msec);
	}

	inline const ::node::DataPacket& GetResponse() const {
		return m_response;
	}

private:
	CCtrlResponse(CCtrlResponse& orig) {}
	CCtrlResponse& operator = (const CCtrlResponse& right) { return *this; }
	friend class CPlayerMathodArgument;
private:
	::node::DataPacket m_response;
	thd::CSpinEvent m_event;
};

class CPlayerMathodArgument : public thd::ThreadBase, public util::PoolBase<CPlayerMathodArgument> {
public:
	CPlayerMathodArgument(
		util::CAutoPointer<CWrapPlayer> pPlayer,
		const ::node::DataPacket& request,
		util::CAutoPointer<CCtrlResponse> pResponse)

		: m_pPlayer(pPlayer)
		, m_request(request)
		, m_pResponse(pResponse)
	{
	}

	virtual bool OnRun() {
		if (m_pResponse.IsInvalid()) {
			::node::DataPacket response;
			SendWorkerNotification(m_request, response, m_pPlayer);
		}
		else {
			SendWorkerNotification(m_request, m_pResponse->m_response, m_pPlayer);
			m_pResponse->m_event.Resume();
		}
		return true;
	}

	virtual void OnShutdown() {}

private:
	util::CAutoPointer<CWrapPlayer> m_pPlayer;
	::node::DataPacket m_request;
	util::CAutoPointer<CCtrlResponse> m_pResponse;
};

class CSelfAsynCallback : public thd::ThreadBase, public util::PoolBase<CSelfAsynCallback> {
public:
	CSelfAsynCallback(
		util::CAutoPointer<CAsynCallback>& cb)

		: m_cb(cb) {}

	virtual bool OnRun() {
		if (m_cb.IsInvalid()) {
			PrintError("@%s  m_cb.IsInvalid()", __FUNCTION__);
			return true;
		}
		m_cb->Invoke(m_response);
		return true;
	}

	virtual void OnShutdown() {}

	::node::DataPacket* MutableResponse() {
		return &m_response;
	}

private:
	::node::DataPacket m_response;
	util::CAutoPointer<CAsynCallback> m_cb;
};

class COtherAsynCallback : public thd::ThreadBase, public util::PoolBase<COtherAsynCallback> {
public:
	COtherAsynCallback(
		util::CAutoPointer<CWrapPlayer>& pReceiver,
		const ::node::DataPacket& request,
		util::CAutoPointer<CAsynCallback>& cb,
		uint64_t senderId)

		: m_pReceiver(pReceiver)
		, m_request(request)
		, m_cb(cb) {}

	virtual bool OnRun() {
		if (m_cb.IsInvalid()) {
			::node::DataPacket response;
			SendWorkerNotification(m_request, response, m_pReceiver);
		} else {
			CSelfAsynCallback* pSelfCallback = new CSelfAsynCallback(m_cb);
			SendWorkerNotification(m_request, *pSelfCallback->MutableResponse(), m_pReceiver);
			thd::ThreadPool.ExecuteTask(pSelfCallback);
		}
		return true;
	}

	virtual void OnShutdown() {}

private:
	util::CAutoPointer<CWrapPlayer> m_pReceiver;
	::node::DataPacket m_request;
	util::CAutoPointer<CAsynCallback> m_cb;
};

class CRemoteAsynCallback : public thd::ThreadBase, public util::PoolBase<CRemoteAsynCallback> {
public:
	CRemoteAsynCallback(
		const ::node::DataPacket& request,
		util::CAutoPointer<CAsynCallback>& cb,
		uint64_t senderId)

		: m_request(request)
		, m_cb(cb) {}

	virtual bool OnRun() {
		if (m_cb.IsInvalid()) {
			::node::DataPacket response;
			SendCachePacketToPlayer(m_request, response);
		} else {
			CSelfAsynCallback* pSelfCallback = new CSelfAsynCallback(m_cb);
			SendCachePacketToPlayer(m_request, *pSelfCallback->MutableResponse());
			thd::ThreadPool.ExecuteTask(pSelfCallback);
		}
		return true;
	}

	virtual void OnShutdown() {}

private:
	::node::DataPacket m_request;
	util::CAutoPointer<CAsynCallback> m_cb;
};

eServerError CPlayerOperate::SendSyn(
	uint64_t senderId,
	const ::node::DataPacket& dataRequest,
	::node::DataPacket& dataResponse,
	const char* file, long line)
{
	if(dataRequest.route() == senderId) {
		return (eServerError)SendWorkerNotification(dataRequest, dataResponse);
	} else {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<CWrapPlayer> pPlayer(pChlMgr->GetPlayer(dataRequest.route()));
		if (pPlayer.IsInvalid()) {
			return SendCachePacketToPlayer(dataRequest, dataResponse);
		} else {
			return (eServerError)SendWorkerNotification(dataRequest, dataResponse);
		}
	}
}

eServerError CPlayerOperate::SendSyn(
	util::CWeakPointer<CWrapPlayer> sender,
	const ::node::DataPacket& dataRequest,
	::node::DataPacket& dataResponse,
	const char* file, long line)
{
	if(!sender.IsInvalid() && sender->GetUserID() == dataRequest.route()) {
		return (eServerError)SendWorkerNotification(dataRequest, dataResponse, sender);
	} else {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<CWrapPlayer> pPlayer(pChlMgr->GetPlayer(dataRequest.route()));
		if (pPlayer.IsInvalid()) {
			return SendCachePacketToPlayer(dataRequest, dataResponse);
		} else {
			return (eServerError)SendWorkerNotification(dataRequest, dataResponse, sender);
		}
	}
}

/////
void CPlayerOperate::SendAsyn(
	uint64_t senderId,
	const ::node::DataPacket& dataRequest,
	util::CAutoPointer<CAsynCallback>& pResponse,
	const char* file, long line)
{
	if(senderId == dataRequest.route()) {
		::node::DataPacket dataResponse;
		SendWorkerNotification(dataRequest, dataResponse);
		pResponse->Invoke(dataResponse);
	} else {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<CWrapPlayer> pPlayer(pChlMgr->GetPlayer(dataRequest.route()));
		if (pPlayer.IsInvalid()) {
			thd::ThreadPool.ExecuteTask(new CRemoteAsynCallback(
				dataRequest, pResponse, senderId));
		} else {
			thd::ThreadPool.ExecuteTask(new COtherAsynCallback(
				pPlayer, dataRequest, pResponse, senderId));
		}
	}
}

void CPlayerOperate::SendAsyn(
	util::CWeakPointer<CWrapPlayer> sender,
	const ::node::DataPacket& dataRequest,
	util::CAutoPointer<CAsynCallback>& pResponse,
	const char* file, long line)
{
	uint64_t senderId = ID_NULL;
	if (!sender.IsInvalid()) {
		senderId = sender->GetUserID();
	}
	if(senderId == dataRequest.route()) {
		::node::DataPacket dataResponse;
		SendWorkerNotification(dataRequest, dataResponse, sender);
		pResponse->Invoke(dataResponse);
	} else {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<CWrapPlayer> pPlayer(pChlMgr->GetPlayer(dataRequest.route()));
		if (pPlayer.IsInvalid()) {
			thd::ThreadPool.ExecuteTask(new CRemoteAsynCallback(
				dataRequest, pResponse, senderId));
		} else {
			thd::ThreadPool.ExecuteTask(new COtherAsynCallback(
				pPlayer, dataRequest, pResponse, senderId));
		}
	}
}
/////

eServerError CPlayerOperate::Post(
	uint64_t senderId,
	const ::node::DataPacket& dataRequest,
	const char* file, long line)
{
	if (dataRequest.route() == senderId) {
		::node::DataPacket dataResponse;
		return (eServerError)SendWorkerNotification(dataRequest, dataResponse);
	} else {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<CWrapPlayer> pPlayer(pChlMgr->GetPlayer(dataRequest.route()));
		if (pPlayer.IsInvalid()) {
			return PostCachePacketToPlayer(dataRequest);
		} else {
			util::CAutoPointer<CCtrlResponse> pCtrlResponse;
			CPlayerMathodArgument* pArg = new CPlayerMathodArgument(
				pPlayer, dataRequest, pCtrlResponse);
			thd::ThreadPool.ExecuteTask(pArg);
			return SERVER_SUCCESS;
		}
	}
}

eServerError CPlayerOperate::Post(
	util::CWeakPointer<CWrapPlayer> sender,
	const ::node::DataPacket& dataRequest,
	const char* file, long line)
{
	if (!sender.IsInvalid() && sender->GetUserID() == dataRequest.route()) {
		::node::DataPacket dataResponse;
		return (eServerError)SendWorkerNotification(dataRequest, dataResponse, sender);
	} else {
		CChannelManager::PTR_T pChlMgr(CChannelManager::Pointer());
		util::CAutoPointer<CWrapPlayer> pPlayer(pChlMgr->GetPlayer(dataRequest.route()));
		if (pPlayer.IsInvalid()) {
			return PostCachePacketToPlayer(dataRequest);
		} else {
			util::CAutoPointer<CCtrlResponse> pCtrlResponse;
			CPlayerMathodArgument* pArg = new CPlayerMathodArgument(
				pPlayer, dataRequest, pCtrlResponse);
			thd::ThreadPool.ExecuteTask(pArg);
			return SERVER_SUCCESS;
		}
	}
}
