/* 
 * File:   IPlayerUnitAdapter.h
 * Author: Jehu Shaw
 * 
 * Created on 2020_5_12, 16:00
 */

#ifndef IPLAYERUNITADAPTER_H
#define	IPLAYERUNITADAPTER_H

#include "Common.h"
#include "WeakPointer.h"

class CPlayer;

namespace node {
	class CreateCharacterRequest;
	class GetCharacterResponse;
}

class IPlayerUnitAdapter {
public:
	virtual ~IPlayerUnitAdapter() {}

	virtual int GetUnitType() const = 0;

	virtual bool RegisterUnit(util::CWeakPointer<CPlayer> pPlayer) = 0;

	virtual int RecursiveCreation(
		IPlayerUnitAdapter** arrAdapters,
		int index, uint64_t userId,
		const ::node::CreateCharacterRequest & req) = 0;

	virtual int OnCreate(
		uint64_t userId,
		const ::node::CreateCharacterRequest & req) = 0;

	virtual int OnCharacterInfo(
		::node::GetCharacterResponse& outResponse,
		uint64_t userId) = 0;
};

template<typename T, int nUnitType>
class CPlayerUnitAdapter : public IPlayerUnitAdapter {
public:
	virtual int GetUnitType() const {
		return nUnitType;
	}

	virtual bool RegisterUnit(util::CWeakPointer<CPlayer> pPlayer) {
		return pPlayer->RegisterUnit((ePlayerUnit)nUnitType, new T(pPlayer));
	}

	virtual int RecursiveCreation(
		IPlayerUnitAdapter** arrAdapters,
		int index, uint64_t userId,
		const ::node::CreateCharacterRequest & req)
	{
		// 创建数据
		int nResult = T::OnCreate(userId, req);
		if (SERVER_SUCCESS == nResult) {
			int nNextIndex = index + 1;
			IPlayerUnitAdapter* pNext = arrAdapters[nNextIndex];
			if (NULL != pNext) {
				int nSubResult = pNext->RecursiveCreation(
					arrAdapters, nNextIndex, userId, req);
				if (SERVER_SUCCESS != nSubResult)
				{
					nResult = nSubResult;
				}
			}
		}
		if (SERVER_SUCCESS != nResult) {
			// 如果有模块创建失败，删除所有已经创建的数据
			T::OnDispose(userId);
		}
		return nResult;
	}

	virtual int OnCreate(
		uint64_t userId,
		const ::node::CreateCharacterRequest & req)
	{
		// 创建数据
		return T::OnCreate(userId, req);
	}

	virtual int OnCharacterInfo(
		::node::GetCharacterResponse& outResponse,
		uint64_t userId)
	{
		return T::OnCharacterInfo(outResponse, userId);
	}
};

#endif /* _IPLAYERUNITADAPTER_H_ */
