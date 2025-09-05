/*
	Code Generate by tool 2020-06-05 16:03:56
*/

#ifndef TEMP_GLOBALTEMPLATE_H
#define TEMP_GLOBALTEMPLATE_H

#include <vector>
#include "Common.h"
#include "TemplateBase.h"

typedef struct GlobalRow {
	// 对应的值
	std::string strValue;
} GlobalRow;

typedef std::map<uint32_t, GlobalRow> GLOBAL_ROWS_T;

class CGlobalTemplate
	: public CTemplateBase
{
public:
	virtual void OnRowData(csv_columns_t& row);

	void GetRowData(csv_columns_t& inRow, uint32_t& outID, GlobalRow& outRow);

	inline const GlobalRow* GetRow(uint32_t nKey)
	{
		GLOBAL_ROWS_T::iterator it = m_rows.find(nKey);
		if(it != m_rows.end()) { return &it->second; }
		return NULL;
	}

protected:
	GLOBAL_ROWS_T m_rows;
};

#endif /* TEMP_GLOBALTEMPLATE_H */
