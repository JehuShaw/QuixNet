/* 
 * File:   ControlArgument.h
 * Author: Jehu Shaw
 * 
 * Created on 2014_7_9, 16:00
 */

#ifndef _CONTROLARGUMENT_H
#define	_CONTROLARGUMENT_H

#include "AgentMethod.h"
#include "worker.pb.h"

class CControlArgument : public evt::ArgumentBase {
public:
	CControlArgument(const ::node::DataPacket& data) 
		: m_data(data) {}

	const ::node::DataPacket& GetData() const { return m_data; }

private:
	const ::node::DataPacket& m_data;
};

#endif	/* _CONTROLARGUMENT_H */

