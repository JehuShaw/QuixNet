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
	// 背包初始化格子数
	int32_t GetInitCellCount() const {
		return m_initCellCount;
	}

	// 背包最大格子数
	int32_t GetMaxCellCount() const {
		return m_maxCellCount;
	}

	// 创建角色的时候初始地图ID
	int32_t GetInitMapID() const {
		return m_initMapID;
	}

	// 任务榜不同难度最多抽取数量
	int32_t GetTaskDifficultFetch() const {
		return m_taskDifficultFetch;
	}

	// 解锁格子数量
	int32_t GetUnlockCellCount() const {
		return m_unlockCount;
	}
	// 需要消耗物品ID 
	uint32_t GetNeedItemCfgID() const {
		return m_needItemCfgId;
	}
	// 需要消耗的物品数量
	int32_t GetNeedItemCount() const {
		return m_needItemCount;
	}
	// 需要消耗的物品每次递增数量
	int32_t GetNeedItemPlus() const {
		return m_needItemPlus;
	}
	// 角色名字限制长度
	int32_t GetCharacterNameMaxSize() const {
		return m_nameMaxSize;
	}

	// 线的中间段玩家占比
	int32_t GetWorldMidRate() const {
		return m_worldMidRate;
	}

	// 私聊一页的记录数量
	int32_t GetPrivateChatPageSize() const {
		return m_priChatPageSize;
	}

	int32_t GetPriChatListKeepSize() const {
		return m_priChatListKeepSize;
	}

	// 系统聊天一页的记录数量
	int32_t GetSystemChatPageSize() const {
		return m_sysChatPageSize;
	}

	// 好友上限
	int32_t GetFriendSizeLimit() const {
		return m_nFriendSizeLimit;
	}

    // 获取小游戏极限时间
    int32_t GetGameLimitTime(uint32_t id, int32_t difficulty) const; 

	const evt::sAtTime& GetRankRewardTime() const {
		return m_tmRankRewardTime;
	}

	// 邮件一页的记录数量
	int32_t GetMailPageSize() const {
		return m_mailPageSize;
	}

    // 角色改名资源消耗
    const ALTER_CONSUME_MAP_TYPE & GetAlterNickConsume() const {
        return m_alterNickConsume;
    }

    bool IsInValidCharacter(const wchar_t wc) const {
        return (m_nickValidCharacter.end() != m_nickValidCharacter.find(wc));
    }

private:
	bool PickKeyValue(uint32_t nKey, const std::string& strValue);

	// 背包初始化格子数
	int32_t m_initCellCount;
	// 背包最大格子数
	int32_t m_maxCellCount;
	// 创建角色的时候初始地图ID
	int32_t m_initMapID;
	// 任务榜不同难度最多抽取数量
	int32_t m_taskDifficultFetch;

	// 解锁格子数量
	int32_t m_unlockCount;
	// 需要消耗的物品ID
	uint32_t m_needItemCfgId;
	// 需要消耗的物品数量
	int32_t m_needItemCount;
	// 需要消耗的物品每次递增数量
	int32_t m_needItemPlus;

	// 角色名字最大限制
	int32_t m_nameMaxSize;

	// 线 [0,m_midPlayerRate) 流畅 [m_midPlayerRate,100)  拥挤 100 爆满
	int32_t m_worldMidRate;

	// 私聊一页的记录数量
	int32_t m_priChatPageSize;
	// 私聊列表保留数量
	int32_t m_priChatListKeepSize;

	// 系统聊天一页的记录数量
	int32_t m_sysChatPageSize;

	// 好友上限
	int32_t m_nFriendSizeLimit;

    // 小游戏极限时间（小于则不合理）game[id] = [[难度1][难度2][难度3]...]
    GAME_TIME_MAP_TYPE m_gamesLimitTime;

	// 排行发奖励时间
	evt::sAtTime m_tmRankRewardTime;

	// 邮件一页的记录数量
	int32_t m_mailPageSize;

    // 角色改名资源消耗
    ALTER_CONSUME_MAP_TYPE m_alterNickConsume;

    // 取名可用字符(unicode)
    std::set<wchar_t> m_nickValidCharacter;
};

#endif /* TEMP_GLOBALCONFIG_H */
