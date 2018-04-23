/* 
 * File:   RankModule.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */
#include "RankModule.h"
#include "NodeDefines.h"
#include "ThreadPool.h"
#include "Log.h"
#include "BodyMessage.h"
#include "ChannelManager.h"
#include "WorkerOperateHelper.h"
#include "CacheOperateHelper.h"
#include "TimestampManager.h"
#include "GuidFactory.h"
#include "CacheRecordManager.h"
#include "AppConfig.h"
#include "FillPacketHelper.h"

#include "RankData.h"
#include "msg_rank_update.pb.h"
#include "msg_rank_request.pb.h"

using namespace mdl;
using namespace util;
using namespace evt;
using namespace thd;

CRankModule::CRankModule(const char* name) : CModule(name), m_rankSet(), m_bLoad(false)
{
    AppConfig::PTR_T pConfig(AppConfig::Pointer());
    m_nRankCacheId = (uint16_t)pConfig->GetInt(APPCONFIG_RANKCACHEID);
	m_rankSet.Init(pConfig->GetInt(APPCONFIG_MAXSIZELIMIT), pConfig->GetInt(APPCONFIG_FLOORLIMIT));

	//util::CAutoPointer<CRankData> pRankData;
	//uint64_t userId = 1;
	//m_rankSet.GetItem(pRankData, userId);
	//m_rankSet.GetRank(userId);
	//m_rankSet.GetSize();
	//RANK_SET_T::NodeRankType items[REQEST_RANK_TOP_SIZE];
	//m_rankSet.GetFirstMoreNodes(items, REQEST_RANK_TOP_SIZE, userId);
	//m_rankSet.GetLastMoreNodes(items, REQEST_RANK_TOP_SIZE, userId);
	//m_rankSet.GetMidMoreNodes(items, REQEST_RANK_TOP_SIZE, userId);
	//m_rankSet.GetNodeByRank(1);
	//m_rankSet.GetFirstMoreNodesByRank(items, REQEST_RANK_TOP_SIZE, 1);
	//m_rankSet.GetLastMoreNodesByRank(items, REQEST_RANK_TOP_SIZE, 1);
	//m_rankSet.GetMidMoreNodesByRank(items, REQEST_RANK_TOP_SIZE, 1);
}

CRankModule::~CRankModule() {
	WaitForDone();
}

void CRankModule::OnRegister(){
   OutputBasic("OnRegister");
}

void CRankModule::OnRemove(){
    OutputBasic("OnRemove");
}

std::vector<int> CRankModule::ListNotificationInterests()
{
	std::vector<int> interests;
    interests.push_back(N_CMD_NODE_REGISTER);
    interests.push_back(N_CMD_C_RANK_UPDATE);
    interests.push_back(N_CMD_C_RANK_REQUEST);
	return interests;
}

IModule::InterestList CRankModule::ListProtocolInterests()
{
	InterestList interests;

	return interests;
}

void CRankModule::HandleNotification(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
    int32_t nCmd = request->GetName();
    switch(nCmd) {
    case N_CMD_C_RANK_UPDATE:
        HandleRankUpdate(request, reply);
        break;
    case N_CMD_C_RANK_REQUEST:
        HandleRankRequest(request, reply);
        break;
	case N_CMD_NODE_REGISTER:
		if((uint16_t)request->GetType() == GetRankCacheId()) {
			if(atomic_cmpxchg8(&m_bLoad, true, false) == (char)false) {
				ThreadPool.ExecuteTask(this);
			}
		}
		break;
	default:
		break;
    }
}

bool CRankModule::Run()
{
	LoadFromDB();
	return false;
}

void CRankModule::LoadFromDB()
{
	util::CValueStream strKeys;
	CResponseRows outRecords;
	CCacheRecordManager::PTR_T pCacheRecordMgr(CCacheRecordManager::Pointer());

	int nCount = LOAD_EACH_PAGE_SIZE;
	if(nCount < 1) {
		nCount = 1;
	}
	int nPage = 0;
	do {	
		CRankData::LoadAllRecord(outRecords, GetRankCacheId(), strKeys, nCount, nPage*nCount);

		int nSize = outRecords.GetSize();
		for(int i = 0; i < nSize; ++i) {
			util::CValueStream tsKeys(outRecords.GetKey(i));
			uint64_t userId; 
			tsKeys.Parse(userId);
			CAutoPointer<CRankData> pRankData(new CRankData);
			util::CValueStream tsValues(outRecords.GetValue(i));
			pRankData->Parse(std::string(tsValues.GetData(), tsValues.GetLength()), outRecords.GetCas(i));
			CAutoPointer<CRankData> pRemoveData;
			if(m_rankSet.InsertItem(pRemoveData, userId, pRankData)) {
				if(pRankData != pRemoveData) {
					strKeys.Serialize(userId);
					pCacheRecordMgr->AddCacheRecord(userId, strKeys, pRankData);
					strKeys.Reset();

					if(!pRemoveData.IsInvalid()) {
						pCacheRecordMgr->RemoveCacheRecord(userId, pRemoveData->ObjectId());
					}
				}
			} else {
				if(!pRemoveData.IsInvalid()) {
					pCacheRecordMgr->RemoveCacheRecord(userId, pRemoveData->ObjectId());
				}
			}
		}

		if(nSize < nCount) {
			break;
		}
		outRecords.Clear();
	} while(true);
}

void CRankModule::HandleRankUpdate(const CWeakPointer<mdl::INotification>& request,
    CWeakPointer<mdl::IResponse>& reply)
{
    CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
    if(pRequest.IsInvalid()) {
        return;
    }

    CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
    if(pResponse.IsInvalid()) {
        return;
    }

    uint64_t userId = pRequest->route();
    
    ::rank::UpdateRankPacket updateRank;
    if(!ParseWorkerData(updateRank, pRequest)) {
        OutputError("!ParseWorkerData userId = "I64FMTD, userId);
        pResponse->set_result(PARSE_PACKAGE_FAIL);
        return;
    }

    util::CAutoPointer<CRankData> pAddData;
	util::CAutoPointer<CRankData> pRemoveData;
	m_rankSet.ReplaceItem(pAddData, pRemoveData, userId, updateRank.score());
	if(pAddData != pRemoveData) {
		if(!pAddData.IsInvalid()) {
			util::CValueStream strKeys;
			strKeys.Serialize(userId);
			CCacheRecordManager::PTR_T pCacheRecordMgr(CCacheRecordManager::Pointer());
			pCacheRecordMgr->AddCacheRecord(userId, strKeys, pAddData);
		}
		if(!pRemoveData.IsInvalid()) {
			CCacheRecordManager::PTR_T pCacheRecordMgr(CCacheRecordManager::Pointer());
			pCacheRecordMgr->RemoveCacheRecord(userId, pRemoveData->ObjectId());
		}
	}

    pResponse->set_result(TRUE);
}

void CRankModule::HandleRankRequest(const CWeakPointer<mdl::INotification>& request,
    CWeakPointer<mdl::IResponse>& reply)
{
    CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
    if(pRequest.IsInvalid()) {
        return;
    }

    CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
    if(pResponse.IsInvalid()) {
        return;
    }

	uint64_t userId = pRequest->route();

    ::rank::RequestRankPacket requestRank;
    if(!ParseWorkerData(requestRank, pRequest)) {
        OutputError("!ParseWorkerData userId = "I64FMTD, userId);
        pResponse->set_result(PARSE_PACKAGE_FAIL);
        return;
    }

    int32_t nRequestRankType = requestRank.rank_type();

    ::rank::RequestRankResultPacket requestRankResult;
    requestRankResult.set_rank_type(nRequestRankType);

    if(REQUEST_RANK_PRIVATE == nRequestRankType) {
        unsigned long nRank;
		int32_t nScore;
		m_rankSet.GetRankAndScore(nRank, nScore, userId);
        ::rank::RankItemPacket* pRankItemPacket = requestRankResult.add_items();
        FillRankItem(*pRankItemPacket, userId, nRank, nScore);
    } else if(REQUEST_RANK_TOP == nRequestRankType) {
		RANK_SET_T::NodeRankType items[REQEST_RANK_TOP_SIZE];
		if(m_rankSet.GetTopRanks(items, REQEST_RANK_TOP_SIZE)) {    
			for(int i = 0; i < REQEST_RANK_TOP_SIZE; ++i) {
				RANK_SET_T::NodeRankType& item = items[i];
				::rank::RankItemPacket* pRankItemPacket = requestRankResult.add_items();
				FillRankItem(*pRankItemPacket, item.pNode->GetValue(),
					item.uRank, item.pNode->GetKey().GetScore());
			}
		}
    } else if(REQUEST_RANK_AROUND == nRequestRankType) {
		RANK_SET_T::NodeRankType items[REQEST_RANK_AROUND_SIZE];
		if(m_rankSet.GetMidMoreNodes(items, REQEST_RANK_AROUND_SIZE, userId)) {
			for(int i = 0; i < REQEST_RANK_AROUND_SIZE; ++i) {
				RANK_SET_T::NodeRankType& item = items[i];
				::rank::RankItemPacket* pRankItemPacket = requestRankResult.add_items();
				FillRankItem(*pRankItemPacket, item.pNode->GetValue(),
					item.uRank, item.pNode->GetKey().GetScore());
			}
		}
	}

    pResponse->set_result(TRUE);

    SerializeWorkerData(pResponse, requestRankResult);
}




