/* 
 * File:   PlayerFactory.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_8_4, 16:00
 */

#ifndef _PLAYERFACTORY_H
#define	_PLAYERFACTORY_H

#include "NodeDefines.h"
#include "AutoPointer.h"
#include "Singleton.h"

class CPlayerBase;

class CPlayerFactory
	: public util::Singleton<CPlayerFactory>
{
public:
    // ����һ���������ʵ������
    util::CAutoPointer<CPlayerBase> CreatePlayer(uint64_t userId);
};

#endif /* _PLAYERFACTORY_H */



