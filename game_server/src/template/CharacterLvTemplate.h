/* 
 * File:   CharacterLvTemplate.h
 * Author: xqx
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef __CHARACTERLVTEMPLATE_H__
#define __CHARACTERLVTEMPLATE_H__

#include <string>
#include "Common.h"
#include "TemplateBase.h"
#include "Singleton.h"

typedef struct CharacterLvRow {
    // 经验
    int nExp;
    // 体力恢复
    int nPowerGain;
    // 体力上限
    int nPowerLimit;
    // 武将等级上限
    int nGeneralLvLimit;
} CharacterLvRow;

typedef std::map<uint32_t, CharacterLvRow> CHARLV_TEMPLATE_ROW_T;

class CCharacterLvTemplate
	: public CTemplateBase
	, public util::Singleton<CCharacterLvTemplate>
{
public:

    virtual void OnRowData(csv_columns_t& row);

    inline const CharacterLvRow* GetRow(uint32_t nId)
    {
        CHARLV_TEMPLATE_ROW_T::iterator it = m_rows.find(nId);
        if(it != m_rows.end()) 
        {
            return &it->second;
        }
        return NULL;
    }

private:
    CHARLV_TEMPLATE_ROW_T m_rows;
};

#endif /* __CHARACTERLVTEMPLATE_H__ */
