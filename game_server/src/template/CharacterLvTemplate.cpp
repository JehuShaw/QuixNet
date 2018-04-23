#include "CharacterLvTemplate.h"
#include "CsvParser.h"


#define FIRST_FIELD_NAME std::string("Lv")
#define SECOND_FIELD_NAME std::string("Exp")
#define THIRD_FIELD_NAME std::string("Power_Gain")
#define FOURTH_FIELD_NAME std::string("Power_Toplimit")
#define FIFTH_FIELD_NAME std::string("General_Lv_Toplimit")

void CCharacterLvTemplate::OnRowData(csv_columns_t& row)
{
    assert(row.size() > 4);
    uint32_t nLevel = (uint32_t)atoi(row[FIRST_FIELD_NAME].c_str());
    CharacterLvRow monsterRow;
    monsterRow.nExp = atoi(row[SECOND_FIELD_NAME].c_str());
    monsterRow.nPowerGain = atoi(row[THIRD_FIELD_NAME].c_str());
    monsterRow.nPowerLimit = atoi(row[FOURTH_FIELD_NAME].c_str());
    monsterRow.nGeneralLvLimit = atoi(row[FIFTH_FIELD_NAME].c_str());

    m_rows[nLevel] = monsterRow;
}

