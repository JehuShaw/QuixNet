/* 
 * File:   MasterCmdManager.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_6_25, 21:20
 */

#ifndef MASTERCMDMANAGER_H
#define MASTERCMDMANAGER_H

#include <string>
#include "Common.h"
#include "NodeModule.h"
#include "Singleton.h"

class CMasterCmdManager
	: public util::Singleton<CMasterCmdManager>
{
public:
	void Init(const std::string& agentServerName) {
		m_agentServerName = agentServerName;
	}

	int SendByServId(uint32_t nServerId, int32_t nCmd) {
		return CNodeModule::SendNodeMessage(nServerId, nCmd);
	}

	int SendByServId(uint32_t nServerId, int32_t nCmd, 
		const ::google::protobuf::Message& request) {
		return CNodeModule::SendNodeMessage(nServerId, nCmd, request);
	}

	int SendByServId(uint32_t nServerId, int32_t nCmd,
		const ::google::protobuf::Message& request,
		::google::protobuf::Message& response) {
		return CNodeModule::SendNodeMessage(nServerId, nCmd, request, response);
	}

	int SendByServId(uint32_t nServerId, int32_t nCmd,
		const std::string& request,
		std::string& response) {
		return CNodeModule::SendNodeMessage(nServerId, nCmd, request, response);
	}

	int SendByUserId(uint64_t nUserId, int32_t nCmd);

	int SendByUserId(
		uint64_t nUserId, int32_t nCmd,
		const ::google::protobuf::Message& message);

	int SendByUserId(
		uint64_t nUserId, int32_t nCmd,
		const ::google::protobuf::Message& request,
		::google::protobuf::Message& response);

	int SendByUserId(
		uint64_t nUserId, int32_t nCmd,
		const std::string& request,
		std::string &response);

    int Restart(uint32_t nServerId);

	int Shutdown(uint32_t nServerId);

	int Erase(uint32_t nServerId);

private:
	std::string m_agentServerName;
};

#endif  // MASTERCMDMANAGER_H
