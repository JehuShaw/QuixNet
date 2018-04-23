/* 
 * File:   MasterCmdManager.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_6_25, 21:20
 */

#ifndef _MASTERCMDMANAGER_H
#define _MASTERCMDMANAGER_H

#include <string>
#include "Common.h"
#include "NodeModule.h"
#include "Singleton.h"

class CMasterCmdManager
	: public util::Singleton<CMasterCmdManager>
{
public:
	int SendByServId(uint16_t nServerId, int32_t nCmd) {
		return CNodeModule::SendNodeMessage(nServerId, nCmd);
	}

	int SendByServId(uint16_t nServerId, int32_t nCmd, 
		const ::google::protobuf::Message& message) {
		return CNodeModule::SendNodeMessage(nServerId, nCmd, message);
	}

	int SendByServId(uint16_t nServerId, int32_t nCmd,
		const std::string& data) {
		return CNodeModule::SendNodeMessage(nServerId, nCmd, data);
	}

	int SendByUserId(
		const std::string& strServerName,
		uint64_t nUserId, int32_t nCmd);

	int SendByUserId(
		const std::string& strServerName,
		uint64_t nUserId, int32_t nCmd,
		const ::google::protobuf::Message& message);

	int SendByUserId(
		const std::string& strServerName,
		uint64_t nUserId, int32_t nCmd,
		const std::string& data);

    int Restart(uint16_t nServerId);

	int Shutdown(uint16_t nServerId);

	int Erase(uint16_t nServerId);
};

#endif  // _MASTERCMDMANAGER_H
