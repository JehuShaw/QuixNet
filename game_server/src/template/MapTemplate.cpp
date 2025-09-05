#include "MapTemplate.h"
#include "CsvParser.h"
#include "NodeDefines.h"
#include "StringResolved.h"

#define FIELD_ID std::string("id")
#define FIELD_TYPE std::string("type")
#define FIELD_SUBTYPE std::string("subType")
#define FIELD_MAXROLENUM std::string("maxRoleNum")
#define FIELD_MAPLEVEL std::string("MapLevel")
#define FIELD_BLOCKLENGTH std::string("blockLength")
#define FIELD_BLOCKWID std::string("blockWid")
#define FIELD_BIRTHPOS std::string("birthPos")
#define FIELD_BIRTHROT std::string("birthRot")
#define FIELD_CAMERAROT std::string("CameraRot")


void CMapTemplate::OnRowData(csv_columns_t& row) {
	assert(row.size() >= 10);

	util::CStringResolved strResolved(csv_field_terminator);
	uint32_t nID;
	MapRow tRow;
	strResolved.CastToType(nID, row[FIELD_ID]);
	strResolved.CastToType(tRow.nType, row[FIELD_TYPE]);
	strResolved.CastToType(tRow.nSubType, row[FIELD_SUBTYPE]);
	strResolved.CastToType(tRow.nMaxRoleNum, row[FIELD_MAXROLENUM]);
	strResolved.CastToType(tRow.nMapLevel, row[FIELD_MAPLEVEL]);
	strResolved.CastToType(tRow.nMapWidth, row[FIELD_BLOCKLENGTH]);
	strResolved.CastToType(tRow.nMapHeight, row[FIELD_BLOCKWID]);
	strResolved.CastToType(tRow.rolePos, row[FIELD_BIRTHPOS]);
	strResolved.CastToType(tRow.roleFace, row[FIELD_BIRTHROT]);
	strResolved.CastToType(tRow.roleCamera, row[FIELD_CAMERAROT]);
	m_rows.insert(MAP_ROWS_T::value_type(nID, tRow));
}


void CMapTemplate::GetRowData(csv_columns_t& inRow, uint32_t& outID, MapRow& outRow) {
	assert(inRow.size() >= 10);

	util::CStringResolved strResolved(csv_field_terminator);
	strResolved.CastToType(outID, inRow[FIELD_ID]);
	strResolved.CastToType(outRow.nType, inRow[FIELD_TYPE]);
	strResolved.CastToType(outRow.nSubType, inRow[FIELD_SUBTYPE]);
	strResolved.CastToType(outRow.nMaxRoleNum, inRow[FIELD_MAXROLENUM]);
	strResolved.CastToType(outRow.nMapLevel, inRow[FIELD_MAPLEVEL]);
	strResolved.CastToType(outRow.nMapWidth, inRow[FIELD_BLOCKLENGTH]);
	strResolved.CastToType(outRow.nMapHeight, inRow[FIELD_BLOCKWID]);
	strResolved.CastToType(outRow.rolePos, inRow[FIELD_BIRTHPOS]);
	strResolved.CastToType(outRow.roleFace, inRow[FIELD_BIRTHROT]);
	strResolved.CastToType(outRow.roleCamera, inRow[FIELD_CAMERAROT]);
}
