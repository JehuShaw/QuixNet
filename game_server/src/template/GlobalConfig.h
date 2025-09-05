/*
 * File:   GlobalConfig.h
 * Author: Jehu Shaw
 *
 * Created on 2020_6_5, 16:06
 */

#ifndef TEMP_GLOBALCONFIG_H
#define TEMP_GLOBALCONFIG_H

#include "GlobalTemplate.h"
#include "Singleton.h"
#include "NodeDefines.h"
#include "TimerEvent.h"

typedef uint16_t LIMIT_TIME_TYPE;
typedef std::vector<LIMIT_TIME_TYPE> LIMIT_TIME_VEC_TYPE;
typedef std::map<uint32_t, std::vector<LIMIT_TIME_TYPE> > GAME_TIME_MAP_TYPE;
typedef std::map<uint32_t, int32_t> ALTER_CONSUME_MAP_TYPE;


class CGlobalConfig 
	: public CGlobalTemplate
	, public util::Singleton<CGlobalConfig>
{
public:
	CGlobalConfig();

	virtual void OnRowData(csv_columns_t& row) {

		uint32_t nKey;
		GlobalRow tRow;
		
		GetRowData(row, nKey, tRow);

		PickKeyValue(nKey, tRow.strValue);
	}

public:
	// ������ʼ��������
	int32_t GetInitCellCount() const {
		return m_initCellCount;
	}

	// ������������
	int32_t GetMaxCellCount() const {
		return m_maxCellCount;
	}

	// ������ɫ��ʱ���ʼ��ͼID
	int32_t GetInitMapID() const {
		return m_initMapID;
	}

	// �����ͬ�Ѷ�����ȡ����
	int32_t GetTaskDifficultFetch() const {
		return m_taskDifficultFetch;
	}

	// ������������
	int32_t GetUnlockCellCount() const {
		return m_unlockCount;
	}
	// ��Ҫ������ƷID 
	uint32_t GetNeedItemCfgID() const {
		return m_needItemCfgId;
	}
	// ��Ҫ���ĵ���Ʒ����
	int32_t GetNeedItemCount() const {
		return m_needItemCount;
	}
	// ��Ҫ���ĵ���Ʒÿ�ε�������
	int32_t GetNeedItemPlus() const {
		return m_needItemPlus;
	}
	// ��ɫ�������Ƴ���
	int32_t GetCharacterNameMaxSize() const {
		return m_nameMaxSize;
	}

	// �ߵ��м�����ռ��
	int32_t GetWorldMidRate() const {
		return m_worldMidRate;
	}

	// ˽��һҳ�ļ�¼����
	int32_t GetPrivateChatPageSize() const {
		return m_priChatPageSize;
	}

	int32_t GetPriChatListKeepSize() const {
		return m_priChatListKeepSize;
	}

	// ϵͳ����һҳ�ļ�¼����
	int32_t GetSystemChatPageSize() const {
		return m_sysChatPageSize;
	}

	// ��������
	int32_t GetFriendSizeLimit() const {
		return m_nFriendSizeLimit;
	}

    // ��ȡС��Ϸ����ʱ��
    int32_t GetGameLimitTime(uint32_t id, int32_t difficulty) const; 

	const evt::sAtTime& GetRankRewardTime() const {
		return m_tmRankRewardTime;
	}

	// �ʼ�һҳ�ļ�¼����
	int32_t GetMailPageSize() const {
		return m_mailPageSize;
	}

    // ��ɫ������Դ����
    const ALTER_CONSUME_MAP_TYPE & GetAlterNickConsume() const {
        return m_alterNickConsume;
    }

    bool IsInValidCharacter(const wchar_t wc) const {
        return (m_nickValidCharacter.end() != m_nickValidCharacter.find(wc));
    }

private:
	bool PickKeyValue(uint32_t nKey, const std::string& strValue);

	// ������ʼ��������
	int32_t m_initCellCount;
	// ������������
	int32_t m_maxCellCount;
	// ������ɫ��ʱ���ʼ��ͼID
	int32_t m_initMapID;
	// �����ͬ�Ѷ�����ȡ����
	int32_t m_taskDifficultFetch;

	// ������������
	int32_t m_unlockCount;
	// ��Ҫ���ĵ���ƷID
	uint32_t m_needItemCfgId;
	// ��Ҫ���ĵ���Ʒ����
	int32_t m_needItemCount;
	// ��Ҫ���ĵ���Ʒÿ�ε�������
	int32_t m_needItemPlus;

	// ��ɫ�����������
	int32_t m_nameMaxSize;

	// �� [0,m_midPlayerRate) ���� [m_midPlayerRate,100)  ӵ�� 100 ����
	int32_t m_worldMidRate;

	// ˽��һҳ�ļ�¼����
	int32_t m_priChatPageSize;
	// ˽���б�������
	int32_t m_priChatListKeepSize;

	// ϵͳ����һҳ�ļ�¼����
	int32_t m_sysChatPageSize;

	// ��������
	int32_t m_nFriendSizeLimit;

    // С��Ϸ����ʱ�䣨С���򲻺���game[id] = [[�Ѷ�1][�Ѷ�2][�Ѷ�3]...]
    GAME_TIME_MAP_TYPE m_gamesLimitTime;

	// ���з�����ʱ��
	evt::sAtTime m_tmRankRewardTime;

	// �ʼ�һҳ�ļ�¼����
	int32_t m_mailPageSize;

    // ��ɫ������Դ����
    ALTER_CONSUME_MAP_TYPE m_alterNickConsume;

    // ȡ�������ַ�(unicode)
    std::set<wchar_t> m_nickValidCharacter;
};

#endif /* TEMP_GLOBALCONFIG_H */
