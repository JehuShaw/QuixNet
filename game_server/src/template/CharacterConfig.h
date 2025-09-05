/*
	Code Generate by tool 2020-09-17 15:25:03
*/

#ifndef TEMP_CHARACTERCONFIG_H
#define TEMP_CHARACTERCONFIG_H

#include <vector>
#include "Common.h"
#include "TemplateBase.h"
#include "Singleton.h"

typedef struct CharacterRow {
	// 性别
	int32_t nGender;
} CharacterRow;

typedef std::map<uint32_t, CharacterRow> CHARACTER_ROWS_T;

class CCharacterConfig
	: public CTemplateBase
	, public util::Singleton<CCharacterConfig>
{
public:
	virtual void OnRowData(csv_columns_t& row);

	inline const CharacterRow* GetRow(uint32_t nID)
	{
		CHARACTER_ROWS_T::iterator it = m_rows.find(nID);
		if(it != m_rows.end()) { return &it->second; }
		return NULL;
	}

private:
	CHARACTER_ROWS_T m_rows;
};

#endif /* TEMP_CHARACTERCONFIG_H */
