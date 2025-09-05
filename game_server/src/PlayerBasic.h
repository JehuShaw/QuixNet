/* 
 * File:   PlayerBasic.h
 * Author: Jehu Shaw
 * 
 * Created on 2020_5_12, 17:10
 */

#ifndef PLAYERBASIC_H
#define	PLAYERBASIC_H

#include "IPlayerUnit.h"
#include "WeakPointer.h"
#include "NodeDefines.h"
#include "PlayerData.h"
#include "CharacterConfig.h"
#include "ObjectSet.h"
#include "CommandManager.h"
#include "ReferObject.h"

class CPlayer;
//class CTitleItem;
class CPlayerTitleData;
class CPlayerHeadData;
class CPlayerHeadFrameData;
//class CHeadStateData;
//class CHeadFrameStateData;
template<class ID_TYPE, class T_OBJ>
class CGeneralState;

enum MCResult;

namespace node {
	class CreateCharacterRequest;
	class GetCharacterResponse;
}

enum ePlayerAttribType {
	PLAYER_ATTRIB_NIL,
	PLAYER_ATTRIB_COIN,
	PLAYER_ATTRIB_GEM,
};


class CPlayerBasic : public IPlayerUnit
{
public:
	CPlayerBasic(const util::CWeakPointer<CPlayer>& pPlayer);

	~CPlayerBasic(void);

	// ��ɫ������ʱ�򴥷�
	static int OnCreate(uint64_t userId, const ::node::CreateCharacterRequest & req);
	// Dispose character callback
	static int OnDispose(uint64_t userId);
	// ��¼��ʱ���ý�ɫ��Ϣ����ʱ���ɫ����û������
	static int OnCharacterInfo(::node::GetCharacterResponse& outResponse, uint64_t userId);
	// ��ɫ��¼��ʱ�򴥷�
	virtual void OnLogin();
	// ������ߵ�ʱ�򴥷�
	virtual void OnLogout();
	// ��ҵ�¼�ɹ�����ͻ��˷��ص�����
	virtual void OnInitClient(game::LoginResponse& outResponse);

    // ������ҽ�ɫ���飬������������, outUpgradeCount �������˼��������û����������0
    int AddExpAndUpgrade(int32_t nExp, int& outUpgradeCount);

    bool AddCoin(int32_t nCoin);

    bool AddGem(int32_t nGem);

    // �ж��������ֵ�Ƿ����
    bool CheckAttribPoint(ePlayerAttribType attribType, int32_t nNeed);
    // �����������ֵ
    bool ConsumeAttribPoint(ePlayerAttribType attribType, int32_t nNeed);
    // ���ӻ��������ֵ
    bool AddAttribPoint(ePlayerAttribType attribType, int32_t nValue);
    /////////////////////////////////////////////////////////////////////////
    // �ж��Ƿ��������, �����õ�ʱ���һ���ڲ�������,����ͻ��ж���Ҫ����
    static bool IsResetTime(const std::string& strLastTime, uint16_t nConfigTime);
	// �������
	util::CAutoPointer<CPlayerData> GetPlayerData() const {
		return m_pPlayerData;     
	}
	// �����ҽ�ɫ
	inline int32_t GetPlayerLevel() const {
		return m_pPlayerData->GetLevel();
	}
    //����ȡ��ɫ����
    inline int32_t GetPlayerExp() const {
        return m_pPlayerData->GetExp();
    }
	// ��ɫ���ñ�ID
	inline uint32_t GetCfgID() const {
		return m_pPlayerData->GetCfgId();
	}
	// ��ɫ��
	inline std::string GetName() const {
		return m_pPlayerData->GetName();
	}
    // ��ɫ�Ա�
    inline int32_t GetGender() const {
        return (NULL != m_pCharacterRow) ? (m_pCharacterRow->nGender) : (GENDER_TYPE_DEFAULT);
    }

	int32_t CalcPlayerFC() const;


private:
    util::CReferObject<CPlayerBasic> m_pThis;
	util::CWeakPointer<CPlayer> m_pPlayer;
	util::CAutoPointer<CPlayerData> m_pPlayerData;
    const CharacterRow * m_pCharacterRow;
};

#endif /* _PLAYERBASIC_H_ */
