#include "Player.h"
#include "NodeDefines.h"
#include "Log.h"
#include "TimestampManager.h"
#include "XqxTable1.h"
#include "msg_node_create_character.pb.h"
#include "PlayerOperateHelper.h"
#include "IPlayerUnitAdapter.h"
#include "ControlCentreStubImpEx.h"
#include "AppConfig.h"


using namespace util;
using namespace evt;


/////////////////////////////////////////////////////////////////////////
IPlayerUnitAdapter* CPlayer::s_arrUnitAdapters[] = {
	////////////////////////////////////////////////////////
	// register player unit 

	///////////////////////////////////////////////////////
	NULL
};
///////////////////////////////////////////////////////////////////////

CPlayer::CPlayer(uint64_t userId) 
    : m_userId(userId)
	, m_worldId(XQXTABLE_INDEX_NIL)
	, m_mapId(ID_NULL)
	, m_playerUnits(PLAYER_UNIT_SIZE? PLAYER_UNIT_SIZE:1)
{
    m_pThis(this);

	// Register unit
	IPlayerUnitAdapter* pNext = s_arrUnitAdapters[0];
	for (int i = 1; NULL != pNext; ++i) {
		pNext->RegisterUnit(m_pThis());
		pNext = s_arrUnitAdapters[i];
	}
}

int CPlayer::OnCreate(uint64_t userId, const ::node::CreateCharacterRequest & req)
{
	int nResult = SERVER_SUCCESS;
	int nIndex = 0;
	IPlayerUnitAdapter* pFrist = s_arrUnitAdapters[nIndex];
	if (NULL != pFrist) {
		if (pFrist->GetUnitType() == PLAYER_UNIT_BASIC) {
			nIndex = 1;
			IPlayerUnitAdapter* pNext = s_arrUnitAdapters[nIndex];
			if (NULL != pNext) {
				nResult = pNext->RecursiveCreation(
					s_arrUnitAdapters, nIndex, userId, req);
				if (SERVER_SUCCESS == nResult) {
					nResult = pFrist->OnCreate(userId, req);
					OutputDebug("OnCreate finished %d nResult = %d userId = " I64FMTD, nIndex, nResult, userId);
				}
			}
		} else {
			nResult = pFrist->RecursiveCreation(
				s_arrUnitAdapters, nIndex, userId, req);
		}
	}
	return nResult;
}

int CPlayer::OnCharacterInfo(::node::GetCharacterResponse& outResponse, uint64_t userId)
{
	int nResult = SERVER_SUCCESS;
	IPlayerUnitAdapter* pNext = s_arrUnitAdapters[0];
	for (int i = 1; NULL != pNext; ++i) {
		nResult = pNext->OnCharacterInfo(outResponse, userId);
		OutputDebug("OnCharacterInfo finished %d nResult = %d userId = " I64FMTD, i, nResult, userId);
		if (SERVER_SUCCESS != nResult) {
			break;
		}
		pNext = s_arrUnitAdapters[i];
	}
	return nResult;
}

uint32_t CPlayer::GetMapIDFromDB(uint64_t userId)
{
	AppConfig::PTR_T pConfig(AppConfig::Pointer());
	std::string strServant(pConfig->GetString(APPCONFIG_SERVANTCONNECT));
	if(strServant.empty()) {
		OutputError("strServant.empty() userId = " I64FMTD, userId);
		return ID_NULL;
	}
	::node::CheckUserResponse checkResponse;
	CControlCentreStubImpEx::PTR_T pCtrlCenStubImpEx(CControlCentreStubImpEx::Pointer());
	if(SERVER_SUCCESS != pCtrlCenStubImpEx->CheckUser(strServant,
		userId, checkResponse))
	{
		OutputError("SERVER_SUCCESS != pCtrlCenStubImpEx->CheckUser userId = " I64FMTD, userId);
		return ID_NULL;
	}
	return checkResponse.mapid();
}

