/* 
 * File:   Player.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef PLAYER_H
#define	PLAYER_H

#include "NodeDefines.h"
#include "PoolBase.h"
#include "ReferObject.h"
#include "IPlayerUnit.h"
#include "TimerManager.h"
#include "SimpleMultiEvent.h"
#include "ArgumentBitStream.h"
#include "Log.h"
#include "IObject.h"
#include "PlayerOperateHelper.h"



class IPlayerUnitAdapter;

namespace node {
	class CreateCharacterRequest;
	class GetCharacterResponse;
	class DataPacket;
}

class CPlayer : public CPlayerBase, public IPlayerUnit, public util::PoolBase<CPlayer>, public util::IObject<uint64_t>
{
public:
	typedef std::vector<util::CAutoPointer<IPlayerUnit> > PLAYER_UNIT_ARRAY;
public:
	CPlayer(uint64_t userId);

	~CPlayer(void) {}

	// Called when the character is created
	static int OnCreate(uint64_t userId, const ::node::CreateCharacterRequest & req);
	// Get Character info
	static int OnCharacterInfo(::node::GetCharacterResponse& outResponse, uint64_t userId);

	virtual uint64_t GetUserID() const {
		return m_userId; 
	}

	virtual uint64_t GetID() const {
		return m_userId;
	}

	uint64_t GetWorldID() const {
		return m_worldId;
	}

	uint32_t GetMapID() const {
		return m_mapId;
	}

	inline void SetMapID(uint32_t mapId) {
		m_mapId = mapId;
	}

	bool RegisterUnit(
		enum ePlayerUnit key,
		const IPlayerUnit* pPlayerUnit,
		bool bDel = true)
	{
		if(pPlayerUnit == this) {
			return false;
		}
		if(key >= (int)m_playerUnits.size()) {
			assert(false);
			return false;
		}
		m_playerUnits[key].SetRawPointer(pPlayerUnit, bDel);
		return true;
	}

	bool UnregisterUnit(enum ePlayerUnit key) {
		if(key >= (int)m_playerUnits.size()) {
			assert(false);
			return false;
		}
		m_playerUnits[key].SetRawPointer(NULL);
		return true;
	}

	inline util::CAutoPointer<IPlayerUnit> GetUnit(enum ePlayerUnit key) {
		if(key >= (int)m_playerUnits.size()) {
			return util::CAutoPointer<IPlayerUnit>();
		}
		return m_playerUnits[key];
	}

	inline util::CAutoPointer<const IPlayerUnit> GetUnit(enum ePlayerUnit key) const {
		if (key >= (int)m_playerUnits.size()) {
			return util::CAutoPointer<const IPlayerUnit>();
		}
		return m_playerUnits[key];
	}

	// Called when the character login
	virtual void OnLogin() {
		int nSize = (int)m_playerUnits.size();
		for(int i = 0; i < nSize; ++i) {
			util::CAutoPointer<IPlayerUnit> pPlayerUint(m_playerUnits[i]);
			if (!pPlayerUint.IsInvalid()) {
				pPlayerUint->OnLogin();
			}
			OutputDebug("OnLogin finished %d userId = " I64FMTD, i, m_userId);
		}
	}
	// Called when the character logout
	virtual void OnLogout() {
		int nSize = (int)m_playerUnits.size();
		for (int i = nSize - 1; i > -1; --i) {
			util::CAutoPointer<IPlayerUnit> pPlayerUint(m_playerUnits[i]);
			if(!pPlayerUint.IsInvalid()) {
				pPlayerUint->OnLogout();
			}
			OutputDebug("OnLogout finished %d userId = " I64FMTD, i, m_userId);
		}
	}
	// Return data where the client login success.
	void OnInitClient(game::LoginResponse& outResponse) {
		int nSize = (int)m_playerUnits.size();
		for (int i = 0; i < nSize; ++i) {
			util::CAutoPointer<IPlayerUnit> pPlayerUint(m_playerUnits[i]);
			if (!pPlayerUint.IsInvalid()) {
				pPlayerUint->OnInitClient(outResponse);
			}
			OutputDebug("OnInitClient finished %d userId = " I64FMTD, i, m_userId);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Set player timeout event.(call once)
	inline bool SetTimeout(id64_t id, unsigned int delay, util::CAutoPointer<util::CallbackBase> method) {
		evt::CTimerManager::PTR_T pTimeMgr(evt::CTimerManager::Pointer());
		return pTimeMgr->SetTimeout(id, delay, method, false, true, true, m_userId);
	}
	// Set player interval event.(call each interval)
	inline bool SetInterval(id64_t id, unsigned int interval, util::CAutoPointer<util::CallbackBase> method, unsigned int delay = 0) {
		evt::CTimerManager::PTR_T pTimeMgr(evt::CTimerManager::Pointer());
		return pTimeMgr->SetInterval(id, interval, method, delay, false, true, true, m_userId);
	}
	// Set player at time event.(call once)
	inline bool SetAtTime(id64_t id, const evt::AtTime& atTime, util::CAutoPointer<util::CallbackBase> method) {
		evt::CTimerManager::PTR_T pTimeMgr(evt::CTimerManager::Pointer());
		return pTimeMgr->SetAtTime(id, atTime, method, false, true, true, m_userId);
	}

	//////////////////////////////////////////////////////////////////////////
	// 角色事件处理
	template<typename T>
	inline void AddEventListener(int cmd, util::CWeakPointer<T> pObj, typename evt::MemberMethodRIP1<T>::MethodType fun) {
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(new evt::MemberMethodRIP1<T>(pObj, fun));
		m_event.AddEventListener(cmd, pMethod);
	}

	inline void AddEventListener(int cmd, typename evt::GlobalMethodRIP1::MethodType fun) {
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(new evt::GlobalMethodRIP1(fun));
		m_event.AddEventListener(cmd, pMethod);
	}

	inline int DispatchEvent(int cmd) {
		return m_event.DispatchEvent(cmd, util::CWeakPointer<evt::ArgumentBase>(), SERVER_SUCCESS, SERVER_FAILURE);
	}

	inline int DispatchEvent(int cmd, const CArgumentBitStream& arg) {
		util::CAutoPointer<evt::ArgumentBase> pArg(&arg, false);
		return m_event.DispatchEvent(cmd, pArg, SERVER_SUCCESS, SERVER_FAILURE);
	}

	inline bool HasEventListener(int cmd) {
		return m_event.HasEventListener(cmd);
	}

	template<class T>
	inline bool HasEventListener(int cmd, util::CWeakPointer<T> pObj, typename evt::MemberMethodRIP1<T>::MethodType fun) {
		evt::MemberMethodRIP1<T> method(pObj, fun);
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(&method, false);
		return m_event.HasEventListener(cmd, pMethod);
	}

	inline bool HasEventListener(int cmd, typename evt::GlobalMethodRIP1::MethodType fun) {
		evt::GlobalMethodRIP1 method(fun);
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(&method, false);
		return m_event.HasEventListener(cmd, pMethod);
	}

	void RemoveEventListener(int cmd) {
		m_event.RemoveEventListener(cmd);
	}

	template<class T>
	inline void RemoveEventListener(int cmd, util::CWeakPointer<T> pObj, typename evt::MemberMethodRIP1<T>::MethodType fun) {
		evt::MemberMethodRIP1<T> method(pObj, fun);
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(&method, false);
		m_event.RemoveEventListener(cmd, pMethod);
	}

	inline void RemoveEventListener(int cmd, typename evt::GlobalMethodRIP1::MethodType fun) {
		evt::GlobalMethodRIP1 method(fun);
		util::CAutoPointer<evt::MethodRIP1Base> pMethod(&method, false);
		m_event.RemoveEventListener(cmd, pMethod);
	}

	inline void Clear() {
		return m_event.Clear();
	}

	// 同步发送命令消息到角色（远程操作必须等待结果）
	int SendSynToPlayer(
		uint64_t targetId, int32_t cmd,
		::google::protobuf::Message* pResponse = NULL)
	{
		return SendSynPlayerCmd(m_pThis(), targetId, cmd, pResponse);
	}
	// 同步发送消息到角色（远程操作必须等待结果）
	int SendSynToPlayer(
		uint64_t targetId, int32_t cmd,
		const ::google::protobuf::Message& request,
		::google::protobuf::Message* pResponse = NULL)
	{
		return SendSynPlayer(m_pThis(), targetId, cmd, request, pResponse);
	}
	// 同步发送数据包到角色（远程操作必须等待结果）
	int SendSynToPlayer(
		const ::node::DataPacket& dataRequest,
		::node::DataPacket& dataResponse)
	{
		return SendSynPlayerPacket(m_pThis(), dataRequest, dataResponse);
	}

	// 异步发送命令消息到角色（如果有返回数据，通过异步回调函数返回）
	void SendAsynToPlayer(
		uint64_t targetId, int32_t cmd,
		util::CAutoPointer<CAsynCallback>& pResponse)
	{
		SendAsynPlayerCmd(m_pThis(), targetId, cmd, pResponse);
	}
	// 异步发送消息到角色（如果有返回数据，通过异步回调函数返回）
	void SendAsynToPlayer(
		uint64_t targetId, int32_t cmd,
		const ::google::protobuf::Message& request,
		util::CAutoPointer<CAsynCallback>& pResponse)
	{
		SendAsynPlayer(m_pThis(), targetId, cmd, request, pResponse);
	}
	// 发送数据包到角色（如果有返回数据，通过异步回调函数返回）
	void SendAsynToPlayer(
		const ::node::DataPacket& dataRequest,
		util::CAutoPointer<CAsynCallback>& pResponse)
	{
		SendAsynPlayerPacket(m_pThis(), dataRequest, pResponse);
	}

	// 发送命令到角色 （远程操作不等待结果）
	int PostToPlayer(
		uint64_t targetId, int32_t cmd)
	{
		return PostPlayerCmd(m_pThis(), targetId, cmd);
	}
	// 发送消息到角色 （远程操作不等待结果）
	int PostToPlayer(
		uint64_t targetId, int32_t cmd,
		const ::google::protobuf::Message& request)
	{
		return PostPlayer(m_pThis(), targetId, cmd, request);
	}
	// 发送数据包到角色（远程操作不等待结果）
	int PostToPlayer(
		const ::node::DataPacket& dataRequest)
	{
		return PostPlayerPacket(m_pThis(), dataRequest);
	}

	// 从数据库获得角色所在地图ID （一般用于获得不在线玩家地图ID）
	static uint32_t GetMapIDFromDB(uint64_t userId);
	//////////////////////////////////////////////////////////////////////////
	// 游戏逻辑部分

private:

	inline void SetWorldID(uint64_t worldId) {
		m_worldId = worldId;
	}

private:
	uint64_t m_userId;
	uint64_t m_worldId;
	uint32_t m_mapId;
    util::CReferObject<CPlayer> m_pThis;
	evt::SimpleMultiEvent<int> m_event;

	PLAYER_UNIT_ARRAY m_playerUnits;
	static IPlayerUnitAdapter* s_arrUnitAdapters[];
};

#endif /* PLAYER_H */
