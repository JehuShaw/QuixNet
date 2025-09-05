/*
	Code Generate by tool 2020-08-21 17:03:24
*/

#ifndef TEMP_MAPCONFIG_H
#define TEMP_MAPCONFIG_H

#include "MapTemplate.h"
#include "Singleton.h"

typedef std::multimap<uint32_t, uint32_t> TYPE_LEVEL_MAP_T;

class CMapConfig : public CMapTemplate , public util::Singleton<CMapConfig>
{
public:

	virtual void OnRowData(csv_columns_t& row)
	{
		MapRow tRow;
		uint32_t nID;

		GetRowData(row, nID, tRow);

		std::pair<MAP_ROWS_T::iterator, bool> pairIB(
			m_rows.insert(MAP_ROWS_T::value_type(nID, tRow)));

		if (pairIB.second) {
			m_levelMap.insert(TYPE_LEVEL_MAP_T::value_type(tRow.nMapLevel, nID));
		}
	}


	// 获取 level 等级以下的地图id [1, level]
	void GetMapIdUnder(int32_t level, std::set<uint32_t> & ids)
	{
		if (!ids.empty()) { ids.clear(); }

		TYPE_LEVEL_MAP_T::iterator rhs = m_levelMap.upper_bound(level);
		if (m_levelMap.end() != rhs) {
			for (TYPE_LEVEL_MAP_T::iterator it(m_levelMap.begin()); it != rhs; ++it) {
                if (ID_NULL == it->second) {
                    continue;
                }
				ids.insert(it->second);
			}
		}
	}

	// 获取等级之间的地图id (lowLevel, highLevel]
	void GetMapIdBetween(int32_t lowLevel, int32_t highLevel, std::set<uint32_t> & ids)
	{
		if (!ids.empty()) { ids.clear(); }
		if (lowLevel >= highLevel) { return; }

		TYPE_LEVEL_MAP_T::const_iterator lhs = m_levelMap.upper_bound(lowLevel);
		TYPE_LEVEL_MAP_T::const_iterator rhs = m_levelMap.upper_bound(highLevel);

		if (lhs != m_levelMap.end()) {
			for (; lhs != rhs; ++lhs) {
				ids.insert(lhs->second);
			}
		}
	}

private:
	TYPE_LEVEL_MAP_T m_levelMap;
};

#endif /* TEMP_MAPCONFIG_H */
