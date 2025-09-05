/*
	Code Generate by tool 2020-06-18 15:04:44
*/

#ifndef TEMP_LEVELEXPCONFIG_H
#define TEMP_LEVELEXPCONFIG_H

#include <vector>
#include "Common.h"
#include "TemplateBase.h"
#include "Singleton.h"

typedef struct LevelExpRow {
	// 到达这个等级所需要的经验值	
	int32_t nExp;
} LevelExpRow;

typedef std::map<int32_t, LevelExpRow> LEVELEXP_ROWS_T;

class CLevelExpConfig
	: public CTemplateBase
	, public util::Singleton<CLevelExpConfig>
{
public:
	virtual void OnRowData(csv_columns_t& row);

	inline const LevelExpRow* GetRow(int32_t nLevel)
	{
		LEVELEXP_ROWS_T::iterator it = m_rows.find(nLevel);
		if (it != m_rows.end()) { return &it->second; }
		return NULL;
	}

private:
	LEVELEXP_ROWS_T m_rows;
};

#endif /* TEMP_LEVELEXPCONFIG_H */
