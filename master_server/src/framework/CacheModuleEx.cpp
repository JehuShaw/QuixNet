#include "CacheModuleEx.h"
#include "ShareDefines.h"


CCacheModuleEx::CCacheModuleEx(
	const std::string& moduleName,
	const std::string& endPoint,
	uint32_t serverId,
	bool routeServer,
	uint64_t routeAddressId,
	const ROUTE_USERIDS_T& routeUserIds)

	: CCacheModule(moduleName, endPoint, serverId, routeServer, routeAddressId, routeUserIds)
{
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

