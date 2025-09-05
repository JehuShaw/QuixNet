/* 
 * File:   ControlEventManager.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2020_12_1, 17:15
 */
#include "ControlEventManager.h"
#include "NodeDefines.h"
#include "Log.h"
#include "WorkerOperateHelper.h"
#include "ControlArgument.h"

using namespace util;
using namespace evt;


CControlEventManager::CControlEventManager()
	: m_arrEvent(C_CMD_CTM_SIZE - NODE_CONTROLCMD_OFFSET)
{
}

bool CControlEventManager::AddEventListener(eControlCMD cmd, const util::CAutoPointer<evt::MethodRIP1Base> method) {
	int nId = cmd - NODE_CONTROLCMD_OFFSET;
	if(nId < 0) {
		OutputError("nId < 0  nId = %d offset = %d", nId, NODE_CONTROLCMD_OFFSET);
		assert(false);
		return false;
	}

	return m_arrEvent.AddEventListener(nId, method);
}

bool CControlEventManager::HasEventListener(eControlCMD cmd) {
	int nId = cmd - NODE_CONTROLCMD_OFFSET;
	if(nId < 0) {
		OutputError("nId < 0  nId = %d offset = %d", nId, NODE_CONTROLCMD_OFFSET);
		assert(false);
		return false;
	}

	return m_arrEvent.HasEventListener(nId);
}

void CControlEventManager::RemoveEventListener(eControlCMD cmd) {
	int nId = cmd - NODE_CONTROLCMD_OFFSET;
	if(nId < 0) {
		OutputError("nId < 0  nId = %d offset = %d", nId, NODE_CONTROLCMD_OFFSET);
		assert(false);
		return;
	}

	m_arrEvent.RemoveEventListener(nId);
}


