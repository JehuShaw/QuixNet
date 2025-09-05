/*
	Code Generate by tool 2020-08-28 09:15:24
*/

#ifndef TEMP_MAPTEMPLATE_H
#define TEMP_MAPTEMPLATE_H

#include <vector>
#include "Common.h"
#include "TemplateBase.h"

typedef struct MapRow {
	// 地图类型
	int32_t nType;
	// 地图子类型
	int32_t nSubType;
	// 分线人数限制
	int32_t nMaxRoleNum;
	// 地图等级
	int32_t nMapLevel;
	// 地图宽 （纵向数量）
	int32_t nMapWidth;
	// 地图高 （横向数量）
	int32_t nMapHeight;
	// 出生点坐标
	std::vector<float> rolePos;
	// 出生朝向
	std::vector<float> roleFace;
	// 相机朝向
	std::vector<float> roleCamera;
} MapRow;

typedef std::map<uint32_t, MapRow> MAP_ROWS_T;

class CMapTemplate
	: public CTemplateBase
{
public:
	virtual void OnRowData(csv_columns_t& row);

	void GetRowData(csv_columns_t& inRow, uint32_t& outID, MapRow& outRow);

	inline const MapRow* GetRow(uint32_t nID)
	{
		MAP_ROWS_T::iterator it = m_rows.find(nID);
		if(it != m_rows.end()) { return &it->second; }
		return NULL;
	}

protected:
	MAP_ROWS_T m_rows;
};

#endif /* TEMP_MAPTEMPLATE_H */
