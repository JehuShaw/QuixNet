#include "LevelExpConfig.h"
#include "CsvParser.h"
#include "NodeDefines.h"
#include "StringResolved.h"

#define FIELD_ID std::string("roleLevel")
#define FIELD_ROLEEXP std::string("roleExp")

void CLevelExpConfig::OnRowData(csv_columns_t& row)
{
	assert(row.size() >= 2);

	util::CStringResolved strResolved(csv_field_terminator);
	int32_t nLevel;
	LevelExpRow tRow;
	strResolved.CastToType(nLevel, row[FIELD_ID]);
	strResolved.CastToType(tRow.nExp, row[FIELD_ROLEEXP]);
	m_rows.insert(LEVELEXP_ROWS_T::value_type(nLevel, tRow));
}
