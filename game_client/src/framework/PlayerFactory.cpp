#include "PlayerFactory.h"
#include "PlayerBase.h"

using namespace util;

CAutoPointer<CPlayerBase> CPlayerFactory::CreatePlayer(uint64_t userId) 
{
    return CAutoPointer<CPlayerBase>();
}

