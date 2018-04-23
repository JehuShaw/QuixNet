#include "TestTemplate.h"
#include "CsvParser.h"
#include "NodeDefines.h"
#include "StringResolved.h"

#define FIELD_ID std::string("ID")
#define FIELD_EFFECT std::string("Effect")
#define FIELD_VALUE std::string("Value")
#define FIELD_VALUE_LV std::string("Value_Lv")
#define FIELD_EFFECTTIMES std::string("EffectTimes")
#define FIELD_TIMEINTERVAL std::string("TimeInterval")


void CTestTemplate::OnRowData(csv_columns_t& row) {
	assert(row.size() >= 6);

	util::CStringResolved strResolved(csv_field_terminator);
	TestRow tRow;
	std::string strKey;
	strResolved.CastToType(strKey, row[FIELD_ID]);
	strResolved.CastToType(tRow.effects, row[FIELD_EFFECT]);
	strResolved.CastToType(tRow.values, row[FIELD_VALUE]);
	strResolved.CastToType(tRow.valueLvRates, row[FIELD_VALUE_LV]);
	strResolved.CastToType(tRow.effectTime, row[FIELD_EFFECTTIMES]);
	strResolved.CastToType(tRow.intervals, row[FIELD_TIMEINTERVAL]);
	m_rows.insert(TEST_ROWS_T::value_type(strKey, tRow));
}
