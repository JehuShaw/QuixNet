/*
	Code Generate by tool 2014-09-01 18:22:07
*/

#ifndef __CCONFIGTEMPLATE_H__
#define __CCONFIGTEMPLATE_H__

#include <vector>
#include "Common.h"
#include "TemplateBase.h"
#include "Singleton.h"

#define CONFIG_PHYPOWER_INTERVAL_NAME std::string("Action_Point_Time")
#define CONFIG_PHYPOWER_RECOVER_NAME std::string("Action_Renew_Value")
#define CONFIG_VERSION std::string("Version")
#define CONFIG_INITIAL_GEM std::string("Initial_Gem")
#define CONFIG_INITIAL_COIN std::string("Initial_Coin")
#define CONFIG_INITIAL_PHYPOWER std::string("Initial_PhyPower")
#define CONFIG_BOSS_DAILYTIMES std::string("Boss_DailyTimes")
#define CONFIG_BOSS_RESETTIMES std::string("Boss_ResetTimes")
#define CONFIG_BOSS_VIPRESETTIMES std::string("Boss_VipResetTimes")
#define CONFIG_FIEFMINE_MAXMINING std::string("FiefMine_MaxMining")
#define CONFIG_FIEFMINE_VIPMAXMINING std::string("FiefMine_VipMaxMining")
#define CONFIG_FIEFMINE_DURATION std::string("FiefMine_Duration")
#define CONFIG_FIEFMINE_MAXMINER std::string("FiefMine_MaxMiner")
#define CONFIG_RESETTIME std::string("Reset_Time")
#define CONFIG_FIEFTRADE_TIMES std::string("FiefTrade_Times")
#define CONFIG_FIEFMINE_MAXTRAINING std::string("FiefMine_MaxTraining")
#define CONFIG_FIEFBARRACK_DURATION std::string("FiefBarrack_Duration")
#define CONFIG_FIEFBARRACK_VIPDURATION std::string("FiefBarrack_VipDuration")
#define CONFIG_FIEFBARRACK_MAXSOLDIER std::string("FiefBarrack_MaxSoldier")
#define CONFIG_ACTIVITY_MAXTIMES std::string("Activity_Num")
#define CONFIG_ACTIVITY_MINLEVEL std::string("Activity_PlayLv")
#define CONFIG_ACTIVITY_RESETTIME std::string("Activity_ResetTime")
#define CONFIG_POINT_TIME std::string("Skill_Point_Time")
#define CONFIG_POINT_LIMIT std::string("Skill_Point_Limit")

typedef std::map<std::string, int32_t> CONFIG_ROWS_T;

class CConfigTemplate
	: public CTemplateBase
	, public util::Singleton<CConfigTemplate>
{
public:
	CConfigTemplate(void);

	virtual void OnRowData(csv_columns_t& row);

	inline int32_t GetValue(const std::string& strName)
	{
		CONFIG_ROWS_T::iterator it = m_rows.find(strName);
		if(it != m_rows.end()) { return it->second; }
#ifdef _DEBUG
        assert(false);
#endif
		return 0;
	}

private:
	CONFIG_ROWS_T m_rows;
};

#endif /* __CCONFIGTEMPLATE_H__ */
