/*
 * File:   PlayerOperateHelper.h
 * Author: Jehu Shaw
 *
 * Created on 2020_9_28 14:55
 */

#ifndef PLAYEROPERATEHELPER_H
#define PLAYEROPERATEHELPER_H

#include "WeakPointer.h"
#include "WorkerOperateHelper.h"
#include "CacheOperateHelper.h"
#include "PlayerBase.h"



// 同步的方式 Send，操作在远程操作的情况下必须严格等待操作结果
#define SendSynPlayer(sender, targetId, cmd, message, pResponse)\
CPlayerOperate::SendSyn(sender, targetId, cmd, message, pResponse, __FILE__, __LINE__)

#define SendSynPlayerCmd(sender, targetId, cmd, pResponse)\
CPlayerOperate::SendSyn(sender, targetId, cmd, pResponse, __FILE__, __LINE__)

#define SendSynPlayerPacket(sender, dataRequest, pDataResponse)\
CPlayerOperate::SendSyn(sender, dataRequest, pDataResponse, __FILE__, __LINE__)

// 异步的方式 Send, 如果有返回数据，通过回掉函数返回。
#define SendAsynPlayer(sender, targetId, cmd, message, pResponse)\
CPlayerOperate::SendAsyn(sender, targetId, cmd, message, pResponse, __FILE__, __LINE__)

#define SendAsynPlayerCmd(sender, targetId, cmd, pResponse)\
CPlayerOperate::SendAsyn(sender, targetId, cmd, pResponse, __FILE__, __LINE__)

#define SendAsynPlayerPacket(sender, dataRequest, pResponse)\
CPlayerOperate::SendAsyn(sender, dataRequest, pResponse, __FILE__, __LINE__)

// Post 操作在远程操作的情况下不等待操作结果
#define PostPlayer(sender, targetId, cmd, message)\
CPlayerOperate::Post(sender, targetId, cmd, message, __FILE__, __LINE__)

#define PostPlayerCmd(sender, targetId, cmd)\
CPlayerOperate::Post(sender, targetId, cmd, __FILE__, __LINE__)

#define PostPlayerPacket(sender, dataRequest)\
CPlayerOperate::Post(sender, dataRequest, __FILE__, __LINE__)


class CAsynCallback {
public:
	virtual void Invoke(const ::node::DataPacket& dataResponse) = 0;
};

class CGAsynCallback : public CAsynCallback {
public:
	typedef void(*Method)(const ::node::DataPacket& dataResponse);
	CGAsynCallback(Method cb) : m_cb(cb) {}
	virtual void Invoke(const ::node::DataPacket& dataResponse) { m_cb(dataResponse); }
private:
	Method m_cb;
};

template<typename T>
class CMAsynCallback : public CAsynCallback {
public:
	typedef void(*Method)(::node::DataPacket& dataResponse);
	CMAsynCallback(util::CWeakPointer<T> obj, Method cb) : m_obj(obj), m_cb(cb) {}
	virtual void Invoke(::node::DataPacket& dataResponse) {
		util::CAutoPointer<T> pObject(m_obj.GetStrong());
		T* ptr = pObject.operator->();
		if(NULL == ptr) {
			assert(NULL != ptr);
			return;
		}
		(ptr->*m_cb)(dataResponse); 
	}
private:
	util::CWeakPointer<T> m_obj;
	Method m_cb;
};


class CPlayerOperate {

public:
	inline static eServerError SendSyn(
		uint64_t senderId, uint64_t targetId, int32_t cmd,
		::google::protobuf::Message* pResponse,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		::node::DataPacket dataResponse;
		eServerError nResult = SendSyn(senderId, dataRequest, dataResponse, file, line);

		if(NULL != pResponse) {
			if (!pResponse->ParseFromString(dataResponse.data())) {
				PrintError("file: %s line: %u @%s !response.ParseFromString"
					, file, line, __FUNCTION__);
				return CACHE_ERROR_PARSE_REQUEST;
			}
		}
		return nResult;
	}

	inline static eServerError SendSyn(
		uint64_t senderId, uint64_t targetId, int32_t cmd,
		const ::google::protobuf::Message& request,
		::google::protobuf::Message* pResponse,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		if (!request.SerializeToString(dataRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !request.SerializeToString"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}

		::node::DataPacket dataResponse;
		eServerError nResult = SendSyn(senderId, dataRequest, dataResponse, file, line);

		if(NULL != pResponse) {
			if (!pResponse->ParseFromString(dataResponse.data())) {
				PrintError("file: %s line: %u @%s !response.ParseFromString"
					, file, line, __FUNCTION__);
				return CACHE_ERROR_PARSE_REQUEST;
			}
		}
		return nResult;
	}

	static eServerError SendSyn(
		uint64_t senderId,
		const ::node::DataPacket& dataRequest,
		::node::DataPacket& dataResponse,
		const char* file, long line);

	inline static eServerError SendSyn(
		util::CWeakPointer<CWrapPlayer> sender,
		uint64_t targetId, int32_t cmd,
		::google::protobuf::Message* pResponse,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		::node::DataPacket dataResponse;
		eServerError nResult = SendSyn(sender, dataRequest, dataResponse, file, line);

		if(NULL != pResponse) {
			if (!pResponse->ParseFromString(dataResponse.data())) {
				PrintError("file: %s line: %u @%s !response.ParseFromString"
					, file, line, __FUNCTION__);
				return CACHE_ERROR_PARSE_REQUEST;
			}
		}
		return nResult;
	}

	inline static eServerError SendSyn(
		util::CWeakPointer<CWrapPlayer> sender,
		uint64_t targetId, int32_t cmd,
		const ::google::protobuf::Message& request,
		::google::protobuf::Message* pResponse,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		if (!request.SerializeToString(dataRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !request.SerializeToString"
				, file, line, __FUNCTION__);
			return CACHE_ERROR_SERIALIZE_REQUEST;
		}

		::node::DataPacket dataResponse;
		eServerError nResult = SendSyn(sender, dataRequest, dataResponse, file, line);

		if(NULL != pResponse) {
			if (!pResponse->ParseFromString(dataResponse.data())) {
				PrintError("file: %s line: %u @%s !response.ParseFromString"
					, file, line, __FUNCTION__);
				return CACHE_ERROR_PARSE_REQUEST;
			}
		}
		return nResult;
	}

	static eServerError SendSyn(
		util::CWeakPointer<CWrapPlayer> sender,
		const ::node::DataPacket& dataRequest,
		::node::DataPacket& dataResponse,
		const char* file, long line);

	//////
	inline static void SendAsyn(
		uint64_t senderId, uint64_t targetId, int32_t cmd,
		util::CAutoPointer<CAsynCallback>& pResponse,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		SendAsyn(senderId, dataRequest, pResponse, file, line);
	}

	inline static void SendAsyn(
		uint64_t senderId, uint64_t targetId, int32_t cmd,
		const ::google::protobuf::Message& request,
		util::CAutoPointer<CAsynCallback>& pResponse,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		if (!request.SerializeToString(dataRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !request.SerializeToString"
				, file, line, __FUNCTION__);
			return;
		}

		SendAsyn(senderId, dataRequest, pResponse, file, line);
	}

	static void SendAsyn(
		uint64_t senderId,
		const ::node::DataPacket& dataRequest,
		util::CAutoPointer<CAsynCallback>& pResponse,
		const char* file, long line);

	inline static void SendAsyn(
		util::CWeakPointer<CWrapPlayer> sender,
		uint64_t targetId, int32_t cmd,
		util::CAutoPointer<CAsynCallback>& pResponse,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		SendAsyn(sender, dataRequest, pResponse, file, line);
	}

	inline static void SendAsyn(
		util::CWeakPointer<CWrapPlayer> sender,
		uint64_t targetId, int32_t cmd,
		const ::google::protobuf::Message& request,
		util::CAutoPointer<CAsynCallback>& pResponse,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		if (!request.SerializeToString(dataRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !request.SerializeToString"
				, file, line, __FUNCTION__);
			return;
		}

		SendAsyn(sender, dataRequest, pResponse, file, line);
	}

	static void SendAsyn(
		util::CWeakPointer<CWrapPlayer> sender,
		const ::node::DataPacket& dataRequest,
		util::CAutoPointer<CAsynCallback>& pResponse,
		const char* file, long line);

	//////

	inline static eServerError Post(
		uint64_t senderId, uint64_t targetId, int32_t cmd,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		return Post(senderId, dataRequest, file, line);
	}

	inline static eServerError Post(
		uint64_t senderId, uint64_t targetId, int32_t cmd,
		const ::google::protobuf::Message& request,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		if (!request.SerializeToString(dataRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !request.SerializeToString"
				, file, line, __FUNCTION__);
		}

		return Post(senderId, dataRequest, file, line);
	}

	static eServerError Post(
		uint64_t senderId,
		const ::node::DataPacket& dataRequest,
		const char* file, long line);

	inline static eServerError Post(
		util::CWeakPointer<CWrapPlayer> sender,
		uint64_t targetId, int32_t cmd,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		return Post(sender, dataRequest, file, line);
	}

	inline static eServerError Post(
		util::CWeakPointer<CWrapPlayer> sender,
		uint64_t targetId, int32_t cmd,
		const ::google::protobuf::Message& request,
		const char* file, long line)
	{
		::node::DataPacket dataRequest;
		dataRequest.set_route_type(ROUTE_BALANCE_USERID);
		dataRequest.set_route(targetId);
		dataRequest.set_cmd(cmd);

		if (!request.SerializeToString(dataRequest.mutable_data())) {
			PrintError("file: %s line: %u @%s !request.SerializeToString"
				, file, line, __FUNCTION__);
		}

		return Post(sender, dataRequest, file, line);
	}

	static eServerError Post(
		util::CWeakPointer<CWrapPlayer> sender,
		const ::node::DataPacket& dataRequest,
		const char* file, long line);
};

#endif /* PLAYEROPERATEHELPER_H */

