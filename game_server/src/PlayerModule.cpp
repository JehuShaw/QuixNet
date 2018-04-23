/*
 * File:   PlayerModule.cpp
 * Author: Jehu Shaw
 *
 * Created on 2014_7_9, 16:00
 */
#include "PlayerModule.h"
#include "NodeDefines.h"
#include "Log.h"
#include "BodyMessage.h"
#include "ChannelManager.h"
#include "WorkerOperateHelper.h"
#include "CacheOperateHelper.h"
#include "msg_client_login.pb.h"
#include "TimestampManager.h"

#include "PlayerData.h"
#include "msg_game_login.pb.h"
#include "Player.h"
#include "FillPacketHelper.h"
#include "ConfigTemplate.h"

#include "CacheRecordManager.h"
#include "msg_game_player_rename.pb.h"
#include "LoggingOperateHelper.h"

using namespace mdl;
using namespace evt;
using namespace util;


#include "WrapRWLock.h"
#include "ScopedWrapRWLock.h"
#include "SpinLock.h"
#include "WrapLock.h"
#include "ScopedWrapLock.h"
#include "SpinRecursiveRWLock.h"

#include "TimerManager.h"

#include "Player.h"

#include "GuidFactory.h"

#include "RecursiveRWLock.h"

#include "TimerManagerHelper.h"

class CTestBase {
public:
	virtual int GetIntValue()const = 0;
	virtual const std::string& GetStrValue() const = 0;

	virtual void SetIntValue(int nValue) = 0;
	virtual void SetStrValue(const std::string& strValue) = 0;
};
class CTest : public CTestBase {
public:
	virtual int GetIntValue()const {
		return m_nValue;
	}
	virtual const std::string& GetStrValue() const {
		return m_strValue;
	}

	virtual void SetIntValue(int nValue) {
		m_nValue = nValue;
	}
	virtual void SetStrValue(const std::string& strValue) {
		m_strValue = strValue;
	}
private:
	int m_nValue;
	std::string m_strValue;
};

class CTest2 {

};

thd::CWrapRWLock<thd::CSpinRWLock, CTest, CTestBase>* g_test;

thd::CWrapLock<thd::CSpinLock, CTest, CTestBase>* g_test2;

thd::CSpinRecursiveRWLock g_srRWLock;

#if PLATFORM == PLATFORM_WIN32
__declspec (naked) unsigned __int64 GetCpuCycle( void )
{
	_asm
	{
		__asm _emit 0x0F
		__asm _emit 0x31
		ret
	}
}
#else
static __inline__ unsigned long long GetCpuCycle(void)
{
 unsigned long long int x;
 __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
 return x;
}
#endif

thd::CSpinRecursiveRWLock* g_pRwLock;

void callBackP1(int& ss) {

};

CPlayerModule::CPlayerModule(const char* name, uint16_t serverRegion)
	: CModule(name), m_u16ServerRegion(serverRegion) {
		m_pThis.SetRawPointer(this, false);

		g_pRwLock = new thd::CSpinRecursiveRWLock;
		//g_rwLock.LockRead();
		thd::CSpinLock spinLock;
		spinLock.TimedLock(100);

		//g_rwLock.UpgradeLockRead();
		//g_rwLock.DegradeLockWrite();
		//g_rwLock.UnlockRead();

		//g_rwLock.LockWrite();

		//g_rwLock.UpgradeLockRead();

		//g_rwLock.UnlockWrite();
		//
		//g_rwLock.LockRead();

		//g_rwLock.LockRead();

		uint64_t nStartTime = GetCpuCycle();
		g_srRWLock.LockRead();
		uint64_t nEndTime = GetCpuCycle();

		int64_t nDifTime = nEndTime - nStartTime;


		g_srRWLock.LockRead();

		SetTimeout(301,CBMethod<CPlayerModule>(m_pThis, &CPlayerModule::TestCallback));

		SetTimeout(300,CBMethod<CPlayerModule>(m_pThis, &CPlayerModule::TestCallback2));

		SetTimeout(400, CBMethod<CPlayerModule>(m_pThis, &CPlayerModule::TestCallback3));

		SetTimeout(500, CBMethod<CPlayerModule>(m_pThis, &CPlayerModule::TestCallback4));

		CPlayer* pPlayer1 = new CPlayer(1);
		CPlayer* pPlayer2 = new CPlayer(2);
		CPlayer* pPlayer3 = new CPlayer(3);
		CPlayer* pPlayer4 = new CPlayer(4);
		CPlayer* pPlayer5 = new CPlayer(5);
		CPlayer* pPlayer6 = new CPlayer(6);
		CPlayer* pPlayer7 = new CPlayer(7);
		CPlayer* pPlayer8 = new CPlayer(8);
		CPlayer* pPlayer9 = new CPlayer(9);

		delete pPlayer1;
		delete pPlayer9;
		delete pPlayer8;
		delete pPlayer7;
		delete pPlayer6;
		delete pPlayer5;
		delete pPlayer4;
		delete pPlayer3;
		delete pPlayer2;


		g_test = new thd::CWrapRWLock<thd::CSpinRWLock, CTest, CTestBase>;
		////////////////////////////////////////////////////////////////////
		// 获得写锁
		thd::CScopedWrapRWLock<CTestBase> swLock(*g_test, 1000);

		////////////////////////////////////////////////////////////////////////
		// 获得读锁
		const thd::CScopedWrapRWLock<CTestBase> srLock(*g_test, 1000);

		///////////////////////////////////////////////////////////////////////
		// 测试

		// 写锁下可以读数据
		swLock->GetStrValue();
		// 写锁下写数据
		swLock->SetIntValue(100);

		//在获得锁的情况下，数据删除将抛出异常
		//delete g_test;

		// 在写锁下获得数据
		util::CScopedPointer<CTestBase> pTest(swLock.GetData());
		pTest->SetStrValue(std::string("Hello world"));

		// 在写锁下读数据
		util::CScopedPointer<const CTestBase> pTest2(swLock.GetData());
		pTest2->GetStrValue();

		// 降级为读锁
		const thd::CScopedWrapRWLock<CTestBase>& cswwLock = swLock.DegradeLockWrite();
		// 降级后写指针失效 ，因此 swwLock->SetIntValue(3); 会抛出异常。
		//swwLock->SetIntValue(3);

		cswwLock->GetIntValue();
		//
		swLock.UpgradeLockRead()->SetIntValue(4);




		g_test2 = new thd::CWrapLock<thd::CSpinLock, CTest, CTestBase>;
		{
			thd::CScopedWrapLock<CTestBase> mutexLock(*g_test2);

			mutexLock->GetIntValue();

			util::CScopedPointer<CTestBase> pTest2(mutexLock.GetData());
			pTest2->SetIntValue(5);
			pTest2->SetStrValue(std::string("hello"));
		}
}

CPlayerModule::~CPlayerModule() {
}

void CPlayerModule::OnRegister(){
    OutputBasic("OnRegister");


}

void CPlayerModule::OnRemove(){
    OutputBasic("OnRemove");
}

std::vector<int> CPlayerModule::ListNotificationInterests()
{
	std::vector<int> interests;

	return interests;
}

IModule::InterestList CPlayerModule::ListProtocolInterests()
{
	InterestList interests;
	interests.push_back(BindMethod<CPlayerModule>(
		P_CMD_C_LOGIN, &CPlayerModule::HandleLogin));

    interests.push_back(BindMethod<CPlayerModule>(
        P_CMD_S_LOGOUT, &CPlayerModule::HandleLogout));

    interests.push_back(BindMethod<CPlayerModule>(
        P_CMD_C_PLAYER_RENAME, &CPlayerModule::HandleRename));

	return interests;
}

void CPlayerModule::HandleNotification(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{

}

void CPlayerModule::HandleLogin(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
	CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
	if(pRequest.IsInvalid()) {
		return;
	}

    // response
    CWeakPointer<::node::DataPacket> pResponse(GetWorkerResponsePacket(reply));
    if(pResponse.IsInvalid()) {
        return;
    }

    uint64_t userId = pRequest->route();

    ::node::LoginRequest loginRequest;
    if(!ParseWorkerData(loginRequest, pRequest)) {
        OutputError("!ParseWorkerData userId = "I64FMTD, userId);
        pResponse->set_result(PARSE_PACKAGE_FAIL);
        return;
    }

    CConfigTemplate::PTR_T pConfigTemp(CConfigTemplate::Pointer());
    uint32_t nSerVersion = (uint32_t)pConfigTemp->GetValue(CONFIG_VERSION);
    if(loginRequest.version() != nSerVersion) {
        OutputError("CONFIG_VERSION_NOT_CONSISTENT userId = "
            I64FMTD" clientVersion = %d serverVersion = %d", userId
            , loginRequest.version(), nSerVersion);
        pResponse->set_result(CONFIG_VERSION_NOT_CONSISTENT);
        return;
    }

	CWeakPointer<CPlayer> pPlayer(GetWorkerPlayer(request));
	if(pPlayer.IsInvalid()) {
		pResponse->set_result(CANNT_FIND_PLAYER);
		return;
	}

    CAutoPointer<CPlayerData> pPlayerData(pPlayer->GetPlayerData());
	util::CValueStream strKeys;
	strKeys.Serialize(userId);
    MCResult nResult = pPlayerData->LoadFromCache(userId, strKeys);
    if(MCERR_OK != nResult && MCERR_NOTFOUND != nResult) {
        OutputError("MCERR_OK != pPlayerData->LoadFromCache nResult = %d"
            " userId = "I64FMTD, nResult, pPlayer->GetUserId());
        // response
        pResponse->set_result(FALSE);
        return;
    }

    if(MCERR_NOTFOUND == nResult) {

        std::string strTime(CTimestampManager::Pointer()->GetLocalDateTimeStr());
        pPlayerData->SetName(std::string("anonymous"));
        pPlayerData->SetAccount(loginRequest.account());
        pPlayerData->SetCreateTime(strTime);
        pPlayerData->SetLevel(1);
		int32_t nInitialGem = pConfigTemp->GetValue(CONFIG_INITIAL_GEM);
		if(nInitialGem > 0) {
			pPlayerData->SetGem(nInitialGem);
		}
		int32_t nInitialCoin = pConfigTemp->GetValue(CONFIG_INITIAL_COIN);
		if(nInitialCoin > 0) {
			pPlayerData->SetCoin(nInitialCoin);
		}

		nResult = pPlayerData->AddToCache(userId, strKeys);
		if(MCERR_OK != nResult) {
			OutputError("MCERR_OK != pPlayerData->AddToCache()"
				" nResult = %d userId = "I64FMTD, nResult, userId);
			pResponse->set_result(FALSE);
			return;
		}
	} else {
		// nResult == MCERR_OK
        if(pPlayerData->GetAccount() != loginRequest.account()) {
            OutputError("pPlayerData->GetAccount() = "I64FMTD" loginRequest.account() = "I64FMTD
                " userId = "I64FMTD, pPlayerData->GetAccount(), loginRequest.account(), userId);
            pResponse->set_result(FALSE);
            return;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    // 添加数据库操作托管
    pPlayer->AddToRecordManager();

    //////////////////////////////////////////////////////////////////////////
    pPlayerData->SetStatus(PLAYER_STATUS_ONLINE);

    game::CharacterPacket characterPacket;

    // character data
    FillPlayerData(characterPacket, *pPlayerData);

    // send to client
    pResponse->set_result(TRUE);
    SerializeWorkerData(pResponse, characterPacket);

	//////////////////////////////////////////////////////////////////////////

	TraceBehavior(userId, pPlayerData->GetAccount(), pPlayerData->GetName(),
		LOG_BEHAVIOR_LOGIN, pPlayerData->GetLevel());
}

void CPlayerModule::HandleLogout(const CWeakPointer<mdl::INotification>& request,
    CWeakPointer<mdl::IResponse>& reply)
{
    CWeakPointer<::node::DataPacket> pRequest(GetWorkerRequestPacket(request));
    if(pRequest.IsInvalid()) {
        return;
    }

	uint64_t userId = pRequest->route();

    CWeakPointer<CPlayer> pPlayer(GetWorkerPlayerSilence(request));
    if(pPlayer.IsInvalid()) {
        return;
    }

    CAutoPointer<CPlayerData> pPlayerData(pPlayer->GetPlayerData());

    std::string strTime(CTimestampManager::Pointer()->GetLocalDateTimeStr());
    pPlayerData->SetOfflineTime(strTime);
    pPlayerData->SetStatus(PLAYER_STATUS_OFFLINE);
    //////////////////////////////////////////////////////////////////////////
    // 退出自动保存数据
    CCacheRecordManager::PTR_T pCacheRecordMgr(CCacheRecordManager::Pointer());
    pCacheRecordMgr->RemoveCacheRecord(userId);
    /////////////////////////////////////////////////////////////////////////

	TraceBehavior(userId, pPlayerData->GetAccount(), pPlayerData->GetName(),
		LOG_BEHAVIOR_LOGOUT, pPlayerData->GetLevel());
}

void CPlayerModule::HandleRename(const CWeakPointer<mdl::INotification>& request,
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
    CWeakPointer<CPlayer> pPlayer(GetWorkerPlayer(request));
    if(pPlayer.IsInvalid()) {
        OutputError("pPlayer.IsInvalid() userId = "I64FMTD, userId);
        pResponse->set_result(CANNT_FIND_PLAYER);
        return;
    }

    ::game::PlayerRenamePacket playerRenamePacket;
    if(!ParseWorkerData(playerRenamePacket, pRequest)) {
        OutputError("!ParseWorkerData userId = "I64FMTD, pPlayer->GetUserId());
        pResponse->set_result(PARSE_PACKAGE_FAIL);
        return;
    }

    CAutoPointer<CPlayerData> pPlayerData(pPlayer->GetPlayerData());

    if(playerRenamePacket.name() == pPlayerData->GetName()) {
        pRequest->set_result(TRUE);
        return;
    }

    std::string strName;
    McDBEscapeString(pPlayer->GetUserId(), playerRenamePacket.name(), strName);
    pPlayerData->SetName(strName);

    pRequest->set_result(TRUE);
}

void CPlayerModule::TestCallback()
{
	if(g_pRwLock)
	g_pRwLock->LockRead();
	if(g_pRwLock)
	g_pRwLock->UpgradeLockRead();

	int ss =3;
	ss += 2;
}

void CPlayerModule::TestCallback2()
{
	if(g_pRwLock)
	g_pRwLock->LockWrite();
	int ss =3;
	ss += 2;
}

void CPlayerModule::TestCallback3()
{
	delete g_pRwLock;
	g_pRwLock = NULL;
	//g_rwLock.UnlockRead();
	//int ss =3;
	//ss += 2;
}

void CPlayerModule::TestCallback4()
{
	//g_rwLock.UnlockWrite();
	int ss =3;
	ss += 2;
}


