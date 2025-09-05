/* 
 * File:   MasterModule.cpp
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */
#include "MasterModule.h"
#include "NodeDefines.h"
#include "Log.h"
#include "MasterServer.h"
#include "MasterChannel.h"
#include "MasterCmdManager.h"

using namespace mdl;
using namespace util;

CMasterModule::CMasterModule(const char* name, uint32_t serverId,
	const std::string& endPoint, const std::string& acceptAddress,
	const std::string& processPath, const std::string& projectName) 

	: INodeControl(name)
{
	CreatChannel(serverId, endPoint, (uint16_t)REGISTER_TYPE_MASTER,
		acceptAddress, processPath, projectName, ID_NULL);
}

void CMasterModule::OnRegister(){
    PrintBasic("CMasterModule OnRegister");
}

void CMasterModule::OnRemove(){
    PrintBasic("CMasterModule OnRemove");
}

std::vector<int> CMasterModule::ListNotificationInterests()
{
	return std::vector<int>({
		N_CMD_MASTER_REGISTER_CACHE,
		N_CMD_MASTER_REMOVE_CACHE,
		N_CMD_MASTER_KEEPTIMEOUT_CACHE,
		N_CMD_SERVANT_NODE_KEEPTIMEOUT
	});
}

IModule::InterestList CMasterModule::ListProtocolInterests()
{
	InterestList interests;
	return interests;
}

void CMasterModule::HandleNotification(const CWeakPointer<INotification>& request,
	CWeakPointer<IResponse>& reply)
{
    int32_t nCmd = request->GetName();
    switch(nCmd) {
    case N_CMD_MASTER_REGISTER_CACHE:
        if(true) {
			CMasterServer::PTR_T pMasterServer(CMasterServer::Pointer());
			if(atomic_cmpxchg8(&pMasterServer->m_bRegistCentre, true, false) == (char)false) {
				// call once
				pMasterServer->RegistControlCentre();
			}
        }
        break;
	case N_CMD_MASTER_REMOVE_CACHE:
	case N_CMD_MASTER_KEEPTIMEOUT_CACHE:
		if(true) {
			CMasterServer::PTR_T pMasterServer(CMasterServer::Pointer());
			if(atomic_cmpxchg8(&pMasterServer->m_bRegistCentre, false, true) == (char)true) {
				// call once
				pMasterServer->UnregistControlCentre();
			}
		}
		break;
	case N_CMD_SERVANT_NODE_KEEPTIMEOUT:
		if(g_bAutoRestart) {
			CMasterCmdManager::PTR_T pMasterCmdMgr(CMasterCmdManager::Pointer());
			uint32_t serverId = request->GetType();
			pMasterCmdMgr->Restart(serverId);
		}
		break;
	default:
		break;
    }
}

bool CMasterModule::CreatChannel(uint32_t serverId, const std::string& endPoint, uint16_t serverType,
	const std::string& acceptAddress, const std::string& processPath, const std::string& projectName,
	uint16_t serverRegion) 
{
	thd::CScopedWriteLock wrLock(m_rwLock);
	if(!m_channel.IsInvalid()) {
		return false;
	}
	m_channel.SetRawPointer(new CMasterChannel);
	if(m_channel.IsInvalid()) {
		return false;
	}
	m_channel->SetServerId(serverId);
	m_channel->SetEndPoint(endPoint);
	m_channel->SetServerType(serverType);
	m_channel->SetAcceptAddress(acceptAddress);
	m_channel->SetProcessPath(processPath);
	m_channel->SetProjectName(projectName);
	m_channel->SetServerRegion(serverRegion);
	return true;
}

bool CMasterModule::RemoveChannel(uint32_t serverId) {
	thd::CScopedWriteLock wrLock(m_rwLock);
	if(m_channel.IsInvalid()) {
		return false;
	}
	if(m_channel->GetServerId() != serverId) {
		return false;
	}
	m_channel.SetRawPointer(NULL);
	return true;
}

int CMasterModule::ChannelCount() const { 
	thd::CScopedReadLock rdLock(m_rwLock);
	if(m_channel.IsInvalid()) {
		return 0;
	}
	return 1; 
}

void CMasterModule::IterateChannel(std::vector<CAutoPointer<IChannelValue> >& outChannels) const {
	thd::CScopedReadLock rdLock(m_rwLock);
	outChannels.push_back(m_channel);
}

CAutoPointer<IChannelValue> CMasterModule::GetLowLoadUserChnl() const {
	thd::CScopedReadLock rdLock(m_rwLock);
	return m_channel;
}

CAutoPointer<IChannelValue> CMasterModule::GetLowLoadByRegion(uint16_t serverRegion) const {
	return CAutoPointer<IChannelValue>();
}

CAutoPointer<IChannelValue> CMasterModule::GetChnlByDirServId(uint32_t serverId) const
{
	thd::CScopedReadLock rdLock(m_rwLock);
	if(m_channel.IsInvalid()) {
		return CAutoPointer<IChannelValue>();
	}
	if(m_channel->GetServerId() != serverId) {
		return CAutoPointer<IChannelValue>();
	}
	return m_channel;
}


