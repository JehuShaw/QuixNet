#include "GlobalTemplate.h"
#include "CsvParser.h"
#include "NodeDefines.h"
#include "StringResolved.h"

#define FIELD_ID std::string("id")
#define FIELD_STRVALUE std::string("strValue")


void CGlobalTemplate::OnRowData(csv_columns_t& row) {
	assert(row.size() >= 2);

	util::CStringResolved strResolved(csv_field_terminator);
	uint32_t nKey;
	GlobalRow tRow;
	strResolved.CastToType(nKey, row[FIELD_ID]);
	strResolved.CastToType(tRow.strValue, row[FIELD_STRVALUE]);
	m_rows.insert(GLOBAL_ROWS_T::value_type(nKey, tRow));
}


void CGlobalTemplate::GetRowData(csv_columns_t& inRow, uint32_t& outID, GlobalRow& outRow) {
	assert(inRow.size() >= 2);

	util::CStringResolved strResolved(csv_field_terminator);
	strResolved.CastToType(outID, inRow[FIELD_ID]);
	strResolved.CastToType(outRow.strValue, inRow[FIELD_STRVALUE]);
}
