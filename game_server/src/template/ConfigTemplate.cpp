#include "ConfigTemplate.h"
#include "CsvParser.h"
#include "NodeDefines.h"

#define FIELD_NAME std::string("Name")
#define FIELD_VALUE std::string("Value")

CConfigTemplate::CConfigTemplate(void) {
    // register the config name
    m_rows[CONFIG_PHYPOWER_INTERVAL_NAME] = 0;
    m_rows[CONFIG_PHYPOWER_RECOVER_NAME] = 0;
    m_rows[CONFIG_PHYPOWER_BUY_VALUE] = 0;
    m_rows[CONFIG_VERSION] = 0;
    m_rows[CONFIG_INITIAL_GEM] = 0;
    m_rows[CONFIG_INITIAL_COIN] = 0;
    m_rows[CONFIG_INITIAL_PHYPOWER] = 0;
    m_rows[CONFIG_BOSS_DAILYTIMES] = 0;
    m_rows[CONFIG_BOSS_RESETTIMES] = 0;
    m_rows[CONFIG_BOSS_VIPRESETTIMES] = 0;
    m_rows[CONFIG_FIEFMINE_MAXMINING] = 0;
    m_rows[CONFIG_FIEFMINE_VIPMAXMINING] = 0;
    m_rows[CONFIG_FIEFMINE_DURATION] = 0;
    m_rows[CONFIG_FIEFMINE_MAXMINER] = 0;
    m_rows[CONFIG_RESETTIME] = 0;
    m_rows[CONFIG_FIEFTRADE_TIMES] = 0;
    m_rows[CONFIG_FIEFMINE_MAXTRAINING] = 0;
    m_rows[CONFIG_FIEFBARRACK_DURATION] = 0;
    m_rows[CONFIG_FIEFBARRACK_VIPDURATION] = 0;
    m_rows[CONFIG_FIEFBARRACK_MAXSOLDIER] = 0;
    m_rows[CONFIG_ACTIVITY_MAXTIMES] = 0;
    m_rows[CONFIG_POINT_TIME] = 0;
    m_rows[CONFIG_POINT_LIMIT] = 0;
    m_rows[CONFIG_CHAPTER_DIFFICULT_TIMES] = 0;
    m_rows[CONFIG_CHAPTER_DIFFICULT_DEBLOCKING] = 0;

    m_rows[CONFIG_COIN_BUY_VALUE] = 0;
}

void CConfigTemplate::OnRowData(csv_columns_t& row) {
	assert(row.size() >= 2);
    CONFIG_ROWS_T::iterator it = m_rows.find(row[FIELD_NAME]);
    if(m_rows.end() != it) {
        it->second = atoi(row[FIELD_VALUE].c_str());
    }
}
