/* 
 * File:   CacheDBServer.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_8_4, 16:00
 */

#ifndef CACHEDBSERVER_H
#define	CACHEDBSERVER_H

#include "AutoPointer.h"
#include "Database.h"

class CCacheDBServer{
public:
	const std::string& GetSchema() const {
		return m_strSchema;
	}
    const std::string& GetDbTable() const {
        return m_strDbTable;
    }
    const std::vector<std::string>& GetKeyColumns() const {
        return m_strKeyColumns;
    }
    const std::vector<std::string>& GetValueColumns() const {
        return m_strValueColumns;
    }
    const std::string& GetCasColumn() const {
        return m_strCasColumn;
    }
    const std::string& GetUniqueIdxNameOnKey() const {
        return m_strUniqueIdxNameOnKey;
    }
    const std::vector<int>& GetLeakyKeyIdxs() const {
        return m_leakyKeyIdxs;
    }
	const std::vector<int>& GetValueKeyIdxs() const {
		return m_valueKeyIdxs;
	}
	const std::vector<int>& GetKeyColumnsType() const {
		return m_keyColumnsType;
	}
	const std::vector<int>& GetValueColumnsType() const {
		return m_valueColumnsType;
	}
private:
    friend class CCacheDBManager;
	util::CAutoPointer<db::Database> m_strSchema;
    std::string m_strDbTable;
    std::vector<std::string> m_strKeyColumns;
    std::vector<std::string> m_strValueColumns;
    std::string m_strCasColumn;
    std::string m_strUniqueIdxNameOnKey;

    std::vector<int> m_leakyKeyIdxs;
	std::vector<int> m_valueKeyIdxs;
	std::vector<int> m_keyColumnsType;
	std::vector<int> m_valueColumnsType;
};


#endif /* CACHEDBSERVER_H */



