#include "CacheModuleEx.h"
#include "ShareDefines.h"


CCacheModuleEx::CCacheModuleEx(void)
	: CCacheModule()
{
}

CCacheModuleEx::CCacheModuleEx(
	const std::string& moduleName,
	const std::string& endPoint,
	uint16_t serverId)

	: CCacheModule(moduleName, endPoint, serverId)
{
	this->moduleName = moduleName;
}

CCacheModuleEx::CCacheModuleEx(
	const std::string& moduleName,
	const std::string& endPoint,
	uint16_t serverId,
	const std::string& acceptAddress,
	const std::string& processPath,
	const std::string& projectName,
	uint16_t serverRegion)

	: CCacheModule(moduleName, endPoint, serverId,
	acceptAddress, processPath, projectName, serverRegion)
{
	this->moduleName = moduleName;
}

bool CCacheModuleEx::FilterProtocolInterest(int nProtocal) {
	if(P_CMD_C_LOGIN == nProtocal
		|| P_CMD_S_LOGOUT == nProtocal)
	{
		// If master server then ignore these commands.
		return true;
	}
	return false;
}

