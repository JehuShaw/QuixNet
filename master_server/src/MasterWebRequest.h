/* 
 * File:   MasterWebRequest.h
 * Author: Jehu Shaw
 *
 * Created on 2014_6_5 15:19
 */

#ifndef MASTERWEBREQUEST_H
#define	MASTERWEBREQUEST_H

#include "Common.h"
#include "HttpRequest.h"
#include "PoolBase.h"

class CMasterWebRequest : public ntwk::HttpRequest, public util::PoolBase<CMasterWebRequest>
{
public:
    static CMasterWebRequest *Create(const char *pUrl,
		unsigned int socketIdx, uint64_t account);
    static void Release(CMasterWebRequest*);

    const char *  GetUrl() const;		

    unsigned int  OnData(const void *, unsigned int bytes);
    unsigned int  OnHead(const char *, unsigned int bytes);
    int           OnProgress(double dltotal, double dlnow);
    void          OnDone(unsigned int dwStatus, const char *);	
	
protected:
private:
    CMasterWebRequest(const char *pUrl, unsigned int socketIdx, uint64_t account);
    ~CMasterWebRequest();

    std::string m_url;
	uint64_t m_account;
	unsigned int m_socketIdx;
};

#endif /* MASTERWEBREQUEST_H */