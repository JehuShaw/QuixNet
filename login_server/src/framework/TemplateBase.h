/* 
 * File:   TemplateBase.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef TEMPLATEBASE_H
#define	TEMPLATEBASE_H

#include <string>
#include <map>
#include "TinyJson.h"

typedef std::map<std::string, std::string> csv_columns_t;

class CTemplateBase
{
public:
    CTemplateBase():m_bInit(false) {}
    ~CTemplateBase(void) {}

    bool Init(const std::string& strPath,
        const std::string& strFileName);

    virtual void OnRowData(csv_columns_t& row) = 0;
    
    bool IsInit()const {
        return m_bInit;
    }

private:
    bool m_bInit;
};

#endif /* TEMPLATEBASE_H */
