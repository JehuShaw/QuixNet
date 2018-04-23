/* 
 * File:   LoginWebRequest.h
 * Author: Jehu Shaw
 *
 * Created on 2014_6_5 15:19
 */
#pragma once

#include "Common.h"
#include "HttpRequest.h"
#include "PoolBase.h"

class CLoginWebRequest : public ntwk::HttpRequest, public util::PoolBase<CLoginWebRequest>
{
public:
    static CLoginWebRequest *Create(const char *pUrl,
		uint64_t account, unsigned int socketIdx);
    static void Release(CLoginWebRequest*);

    const char *  GetUrl() const;	

    unsigned int  OnData(const void *, unsigned int bytes);
    unsigned int  OnHead(const char *, unsigned int bytes);
    int           OnProgress(double dltotal, double dlnow);
    void          OnDone(unsigned int dwStatus, const char *);	
	
private:
    CLoginWebRequest(const char *pUrl,
		uint64_t account, unsigned int socketIdx);
    ~CLoginWebRequest();

    std::string m_url;
	uint64_t m_account;
	unsigned int m_socketIdx;
};

