#include "CharacterConfig.h"
#include "CsvParser.h"
#include "NodeDefines.h"
#include "StringResolved.h"

#define FIELD_ID std::string("id")
#define FIELD_SEX std::string("sex")


void CCharacterConfig::OnRowData(csv_columns_t& row) {
	assert(row.size() >= 2);

	util::CStringResolved strResolved(csv_field_terminator);
	uint32_t nID;
	CharacterRow tRow;
	strResolved.CastToType(nID, row[FIELD_ID]);
	strResolved.CastToType(tRow.nGender, row[FIELD_SEX]);
	m_rows.insert(CHARACTER_ROWS_T::value_type(nID, tRow));
}
