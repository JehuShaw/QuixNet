#include "GlobalConfig.h"
#include "StringResolved.h"
#include "Log.h"
#include "TimerManager.h"
#include "Utf8.h"

const static char split_field_terminator = '|';

enum eGlobalCFGKey {
	G_CFG_NIL = 0,
	G_CFG_BACKPACK_CELL = 50004,
	G_CFG_INIT_MAPID = 50005,
	G_CFG_TASK_DIFFICULT_FETCH = 50006,
	G_CFG_CHARACTER_NAME_SIZE = 50007,
	G_CFG_UNLOCK_BACKPACK_CELL = 50009,
	G_CFG_WORLD_PLAYER_RATE = 50012,
	G_CFG_PRIVATE_CHAT_PAGE_SIZE = 50018,
	G_CFG_PRI_CHAT_LIST_KEEP_SIZE = 50019,
	G_CFG_SYSTEM_CHAT_PAGE_SIZE = 50020,
	G_CFG_FRIEND_SIZE_LIMIT = 50021,
	G_CFG_GAMES_LIMIT_TIME = 50023,
	G_CFG_RANK_REWARD_TIME = 50027,
	G_CFG_MAIL_PAGE_SIZE = 50036,
    G_CFG_NICK_VALID_CHARACTER = 50037,
	G_CFG_ALTER_NICK_CONSUME = 50038,
};

CGlobalConfig::CGlobalConfig()
    : m_initCellCount(0)
    , m_maxCellCount(0)
    , m_initMapID(0)
    , m_taskDifficultFetch(0)
    , m_unlockCount(0)
    , m_needItemCfgId(0)
    , m_needItemCount(0)
    , m_needItemPlus(0)
    , m_nameMaxSize(0)
    , m_worldMidRate(0)
	, m_priChatPageSize(1)
	, m_priChatListKeepSize(1)
	, m_sysChatPageSize(1)
	, m_nFriendSizeLimit(1)
	, m_mailPageSize(1)
{
    m_alterNickConsume.clear();
}

int32_t CGlobalConfig::GetGameLimitTime(uint32_t id, int32_t difficulty) const
{
    if (0 >= difficulty) {
        return 0;
    }

    int sec = 0;
    GAME_TIME_MAP_TYPE::const_iterator it = m_gamesLimitTime.find(id);
    if (it == m_gamesLimitTime.end()) {
        OutputError("Cannot find this game id %d, maybe global cfg not define.", id);
        return sec;
    }
    if ((int32_t)it->second.size() >= difficulty) {
        sec = it->second[difficulty - 1];
    }

    return sec;
}

// 请在下面设置服务端需要加载的全局配置
bool CGlobalConfig::PickKeyValue(uint32_t nKey, const std::string& strValue)
{
	switch (nKey)
	{
	case G_CFG_BACKPACK_CELL: {
		int32_t arr[2] = { 0,0 };
		util::CStringResolved strResolved(split_field_terminator);
		strResolved.CastToType(arr, strValue);
		m_initCellCount = arr[0];
		m_maxCellCount = arr[1];
		return true;
	}
	case G_CFG_INIT_MAPID: {
		util::CStringResolved strResolved;
		strResolved.CastToType(m_initMapID, strValue);
		return true;
	}
	case G_CFG_TASK_DIFFICULT_FETCH: {
		util::CStringResolved strResolved;
		strResolved.CastToType(m_taskDifficultFetch, strValue);
		return true;
	}
	case G_CFG_CHARACTER_NAME_SIZE: {
		util::CStringResolved strResolved;
		strResolved.CastToType(m_nameMaxSize, strValue);
		return true;
	}
	case G_CFG_UNLOCK_BACKPACK_CELL: {
		int32_t arr[4] = { 0,0,0,0 };
		util::CStringResolved strResolved(split_field_terminator);
		strResolved.CastToType(arr, strValue);
		m_unlockCount = arr[0];
		m_needItemCfgId = arr[1];
		m_needItemCount = arr[2];
		m_needItemPlus = arr[3];
		return true;
	}
	case G_CFG_WORLD_PLAYER_RATE: {
		m_worldMidRate = atoi(strValue.c_str());
		return true;
	}
	case G_CFG_PRIVATE_CHAT_PAGE_SIZE:{
		m_priChatPageSize = atoi(strValue.c_str());
		return true;
	}
	case G_CFG_PRI_CHAT_LIST_KEEP_SIZE: {
		m_priChatListKeepSize = atoi(strValue.c_str());
		return true;
	}
	case G_CFG_SYSTEM_CHAT_PAGE_SIZE: {
		m_sysChatPageSize = atoi(strValue.c_str());
		return true;
	}
	case G_CFG_FRIEND_SIZE_LIMIT: {
		m_nFriendSizeLimit = atoi(strValue.c_str());
		return true;
	}
    case G_CFG_GAMES_LIMIT_TIME: {
        tiny::Json diffs;
        util::CStringResolved strResolved(csv_field_terminator);
        strResolved.CastToType(diffs, strValue);
        int count = diffs.Count();

        tiny::JArray arr;
        for (int i = 0; i < count; ++i) {
            if (!diffs.GetAt(arr, i)) {
                continue;
            }

            int fieldCnt = arr.Count();
            if (1 >= fieldCnt) {
                continue; // 必须有id和具体难度阶梯
            }

            uint32_t gameID = ID_NULL;
            if (arr.GetAt(gameID, 0)) {
                std::pair<GAME_TIME_MAP_TYPE::iterator, bool> pairIB(
                m_gamesLimitTime.insert(GAME_TIME_MAP_TYPE::value_type(gameID, LIMIT_TIME_VEC_TYPE())));

                if (!pairIB.second) {
                    OutputError("G_CFG_GAMES_LIMIT_TIME check same game id field，game id %d.", gameID);
                    continue;
                }

                for (int field = 1; field < fieldCnt; ++field) {
                    LIMIT_TIME_TYPE sec;
                    if (arr.GetAt(sec, field)) {
                        pairIB.first->second.push_back(sec);
                    }
                }
            }
        }
        return true;
    }
	case G_CFG_RANK_REWARD_TIME: {
		memset(&m_tmRankRewardTime, 0, sizeof(m_tmRankRewardTime));
		std::string arr[2];
		util::CStringResolved strResolved(split_field_terminator);
		strResolved.CastToType(arr, strValue);
		m_tmRankRewardTime.week = atoi(arr[0].c_str());
		sscanf(arr[1].c_str(), "%2c:%2c:%2c", 
			&m_tmRankRewardTime.hour,
			&m_tmRankRewardTime.minute, 
			&m_tmRankRewardTime.second);
		return true;
	}
	case G_CFG_MAIL_PAGE_SIZE: {
		m_mailPageSize = atoi(strValue.c_str());
		return true;
	}
    case G_CFG_ALTER_NICK_CONSUME: {
        int arr[2] = {0, 0};
        util::CStringResolved strResolved(split_field_terminator);
        strResolved.CastToType(arr, strValue);
        m_alterNickConsume.insert(ALTER_CONSUME_MAP_TYPE::value_type(arr[0], arr[1]));
        return true;
    }
    case G_CFG_NICK_VALID_CHARACTER: {
        std::vector<std::string> validStr;
        util::CStringResolved strResolved(split_field_terminator);
        strResolved.CastToType(validStr, strValue);

        for (std::vector<std::string>::const_iterator cit(validStr.cbegin()); cit != validStr.cend(); ++cit) {
            wchar_t swzBuf = 0;
            util::UTF8ToUNICODE((util::custr)(cit->data()), cit->length(), &swzBuf, 1);
            m_nickValidCharacter.insert(swzBuf);
        }
        return true;
    }
	default:
		break;
	}
	return false;

}
