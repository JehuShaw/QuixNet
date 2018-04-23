/*
	Code Generate by tool 2016-07-31 20:34:58
*/

#ifndef __CTESTTEMPLATE_H__
#define __CTESTTEMPLATE_H__

#include <vector>
#include "Common.h"
#include "TemplateBase.h"
#include "Singleton.h"

typedef struct TestRow {
	// 状态效果
	std::vector<uint32_t> effects;
	// 状态效果值
	std::vector<int32_t> values;
	// 技能等级换算状态值配比参数值，状态效果值=Value+（技能等级-1）×Value_Lv
	std::vector<float> valueLvRates;
	// 状态持续时间
	int32_t effectTime;
	// 状态间隔
	std::vector<int32_t> intervals;
} TestRow;

typedef std::map<std::string, TestRow> TEST_ROWS_T;

class CTestTemplate
	: public CTemplateBase
	, public util::Singleton<CTestTemplate>
{
public:
	virtual void OnRowData(csv_columns_t& row);

	inline const TestRow* GetRow(std::string strKey)
	{
		TEST_ROWS_T::iterator it = m_rows.find(strKey);
		if(it != m_rows.end()) { return &it->second; }
		return NULL;
	}

private:
	TEST_ROWS_T m_rows;
};

#endif /* __CTESTTEMPLATE_H__ */
