/*
 * File:   RankSetMT.h
 * Author: Jehu Shaw
 *
 * Created on 2016_5_13, 17:38
 */

#ifndef __RANKSETMT_H_
#define __RANKSETMT_H_

#include <map>
#include "SkipList.h"
#include "TimeSeries.h"
#include "IRankData.h"
#include "SpinRWLock.h"

namespace util {

template <class IDType, class ScoreType, class RankDataType>
class CRankSetMT
{
	class CSkipListKey
	{
	public:
		CSkipListKey()
			: m_score(0)
			, m_u32Time(0)
			, m_u32Count(0) {
		}

		CSkipListKey(ScoreType score)
			: m_score(score)
			, m_u32Time(0)
			, m_u32Count(0) {
		}

		CSkipListKey(const CSkipListKey& orig)
			: m_score(orig.m_score)
			, m_u32Time(orig.m_u32Time)
			, m_u32Count(orig.m_u32Count) {
		}

		CSkipListKey(const util::CAutoPointer<IRankData<ScoreType> >& inRankData) {
			m_score = inRankData->GetScore();
			m_u32Time = inRankData->GetTime();
			m_u32Count = inRankData->GetCount();
		}

		bool operator < (const CSkipListKey& right) const {
			// max score up rank
			if(m_score > right.m_score) {
				return true;
			} else if(m_score == right.m_score) {
				if(m_u32Time > right.m_u32Time) {
					return true;
				} else if(m_u32Time == right.m_u32Time) {
					return m_u32Count > right.m_u32Count;
				}
			}
			return false;
		}

		bool operator == (const CSkipListKey& right) const {
			return m_score == right.m_score
				&& m_u32Time == right.m_u32Time
				&& m_u32Count == right.m_u32Count;
		}

		CSkipListKey& operator = (const CSkipListKey& right) {
			m_score = right.m_score;
			m_u32Time = right.m_u32Time;
			m_u32Count = right.m_u32Count;
			return *this;
		}

		inline void CopyTo(util::CAutoPointer<IRankData<ScoreType> >& outRankData) const {
			outRankData->SetScore(m_score);
			outRankData->SetTime(m_u32Time);
			outRankData->SetCount(m_u32Count);
		}

		inline ScoreType GetScore() const {
			return m_score;
		}

	private:
		template <class FIDType, class FScoreType, class FRankDataType> friend class CRankSetMT;
		ScoreType m_score;
		uint32_t m_u32Time;
		uint32_t m_u32Count;
	};

public:
	typedef typename CSkipList<CSkipListKey, IDType>::NodeRankType NodeRankType;
	typedef typename CSkipList<CSkipListKey, IDType>::NodeType NodeType;

public:
	CRankSetMT() : m_uLimitSize(0), m_scoreFloor(0) {}
	CRankSetMT(unsigned long uLimitSize, ScoreType scoreFloor)
		: m_uLimitSize(uLimitSize), m_scoreFloor(scoreFloor) {}
	~CRankSetMT(){}

public:
	inline void Init(unsigned long uLimitSize, ScoreType scoreFloor) {
		thd::CScopedWriteLock wrLock(m_rwLock);
		m_uLimitSize = uLimitSize;
		m_scoreFloor = scoreFloor;
	}
	// 增加一项
	bool InsertItem(IDType id, util::CAutoPointer<RankDataType>& inRankData);
	// 增加一项并且返回删除项
	bool InsertItem(util::CAutoPointer<RankDataType>& outRemoveData, IDType id, util::CAutoPointer<RankDataType>& inRankData);
	// 增加或者更新一项
	bool ReplaceItem(IDType id, ScoreType score);
	// 增加或者更新一项并返回新增项和删除项
	bool ReplaceItem(util::CAutoPointer<RankDataType>& outAddData, util::CAutoPointer<RankDataType>& outRemoveData, IDType id, ScoreType score);
	// 删除一项
	bool DelItem(IDType id);
	// 删除一项并返回删除的项
	bool DelItem(util::CAutoPointer<RankDataType>& outRankData, IDType id);
	// 获取一项
	bool GetItem(util::CAutoPointer<RankDataType>& outRankData, IDType id);
	// 获取排名和分值
	bool GetRankAndScore(unsigned long& outRank, ScoreType& outScore, IDType id);
	// 获得排名
	unsigned long GetRank(IDType id);
	// 获取某个排序管理器的排名, 只取top排名
	bool GetTopRanks(NodeRankType* outNodes, uint32_t n32Count);
	// 获取排序容器大小
	unsigned long GetSize() const;
	// 更排名获得节点
	inline NodeType* GetNodeByRank(unsigned long rank) {
		thd::CScopedReadLock rdLock(m_rwLock);
		return m_skipList.GetNodeByRank(rank);
	}
	// 从指定排名firstRank 开始获取之后 n32Count （指定的数量包含 firstRank）指定个数的节点集
	inline bool GetFirstMoreNodesByRank(NodeRankType* outNodes, uint32_t u32Count, unsigned long firstRank) {
		thd::CScopedReadLock rdLock(m_rwLock);
		return m_skipList.GetFirstMoreNodesByRank(outNodes, u32Count, firstRank);
	}
	// 从指定排名lastRank 开始获取之前 n32Count （指定的数量包含 lastRank）指定个数的节点集
	inline bool GetLastMoreNodesByRank(NodeRankType* outNodes, uint32_t u32Count, unsigned long lastRank) {
		thd::CScopedReadLock rdLock(m_rwLock);
		return m_skipList.GetLastMoreNodesByRank(outNodes, u32Count, lastRank);
	}
	// 从指定排名midRank 开始获取之前和之后 n32Count/2 （指定的数量包含 midRank）指定个数的节点集
	inline bool GetMidMoreNodesByRank(NodeRankType* outNodes, uint32_t u32Count, unsigned long midRank, bool reverse = false) {
		thd::CScopedReadLock rdLock(m_rwLock);
		return m_skipList.GetMidMoreNodesByRank(outNodes, u32Count, midRank, reverse);
	}
	// 从指定id 的排行开始获取之后 n32Count （指定的数量包含 id）指定个数的节点集
	bool GetFirstMoreNodes(NodeRankType* outNodes, uint32_t u32Count, IDType id);
	// 从指定id 的排名开始获取之前 n32Count （指定的数量包含 id）指定个数的节点集
	bool GetLastMoreNodes(NodeRankType* outNodes, uint32_t u32Count, IDType id);
	// 从指定id 的排名开始获取之前和之后 n32Count/2 （指定的数量包含 id）指定个数的节点集
	bool GetMidMoreNodes(NodeRankType* outNodes, uint32_t u32Count, IDType id, bool reverse = false);

private:
	typedef CSkipList<CSkipListKey, IDType> SKIP_LIST_T;
	typedef std::map<IDType, util::CAutoPointer<IRankData<ScoreType> > > ID_TO_RANKDAT_T;
	SKIP_LIST_T m_skipList;
	ID_TO_RANKDAT_T m_id2RankData;
	util::CTimeSeries m_timeSeries;
	unsigned long m_uLimitSize;
	ScoreType m_scoreFloor;
	thd::CSpinRWLock m_rwLock;
};

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::InsertItem(IDType id, util::CAutoPointer<RankDataType>& inRankData) {
	thd::CScopedWriteLock wrLock(m_rwLock);
	CSkipListKey slKey(inRankData);
	if(0 != m_scoreFloor && slKey.GetScore() < m_scoreFloor) {
		DelItem(id);
		return false;
	}
	std::pair<typename ID_TO_RANKDAT_T::iterator, bool> pairIB(m_id2RankData.insert(
		typename ID_TO_RANKDAT_T::value_type(id, inRankData)));
	if(pairIB.second) {
		m_skipList.AddNode(slKey, id);
		if(0 != m_uLimitSize && m_skipList.Size() > m_uLimitSize) {
			typename SKIP_LIST_T::RIterator iter(m_skipList.RBegin());
			if(m_skipList.REnd() != iter) {
				DelItem(iter->GetValue());
			}
		}
		return true;
	}
	return false;
}

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::InsertItem(util::CAutoPointer<RankDataType>& outRemoveData, IDType id, util::CAutoPointer<RankDataType>& inRankData) {
	thd::CScopedWriteLock wrLock(m_rwLock);
	CSkipListKey slKey(inRankData);
	if(0 != m_scoreFloor && slKey.GetScore() < m_scoreFloor) {
		DelItem(outRemoveData, id);
		return false;
	}
	std::pair<typename ID_TO_RANKDAT_T::iterator, bool> pairIB(m_id2RankData.insert(
		typename ID_TO_RANKDAT_T::value_type(id, inRankData)));
	if(pairIB.second) {
		m_skipList.AddNode(slKey, id);
		if(0 != m_uLimitSize && m_skipList.Size() > m_uLimitSize) {
			typename SKIP_LIST_T::RIterator iter(m_skipList.RBegin());
			if(m_skipList.REnd() != iter) {
				DelItem(outRemoveData, iter->GetValue());
			}
		}
		return true;
	}
	return false;
}

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::ReplaceItem(IDType id, ScoreType score) {
	thd::CScopedWriteLock wrLock(m_rwLock);
	if(0 != m_scoreFloor && score < m_scoreFloor) {
		DelItem(id);
		return false;
	}
	typename ID_TO_RANKDAT_T::iterator iter(m_id2RankData.lower_bound(id));
	if(m_id2RankData.end() != iter && iter->first == id) {
		if(score == iter->second->GetScore()) {
			return false;
		}
		CSkipListKey slNewKey(score);
		m_timeSeries.Generate(slNewKey.m_u32Time, slNewKey.m_u32Count);
		m_skipList.UpdateNode(iter->second, id, slNewKey);
		slNewKey.CopyTo(iter->second);
	} else {
		CSkipListKey slKey(score);
		m_timeSeries.Generate(slKey.m_u32Time, slKey.m_u32Count);
		util::CAutoPointer<IRankData<ScoreType> > rankData(new RankDataType);
		slKey.CopyTo(rankData);
		m_id2RankData.insert(iter, ID_TO_RANKDAT_T::value_type(id, rankData));
		m_skipList.AddNode(slKey, id);
		if(0 != m_uLimitSize && m_skipList.Size() > m_uLimitSize) {
			typename SKIP_LIST_T::RIterator iter(m_skipList.RBegin());
			if(m_skipList.REnd() != iter) {
				DelItem(iter->GetValue());
			}
		}
	}
	return true;
}

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::ReplaceItem(util::CAutoPointer<RankDataType>& outAddData, util::CAutoPointer<RankDataType>& outRemoveData, IDType id, ScoreType score) {
	thd::CScopedWriteLock wrLock(m_rwLock);
	if(0 != m_scoreFloor && score < m_scoreFloor) {
		DelItem(outRemoveData, id);
		return false;
	}
	typename ID_TO_RANKDAT_T::iterator iter(m_id2RankData.lower_bound(id));
	if(m_id2RankData.end() != iter && iter->first == id) {
		if(score == iter->second->GetScore()) {
			return false;
		}
		CSkipListKey slNewKey(score);
		m_timeSeries.Generate(slNewKey.m_u32Time, slNewKey.m_u32Count);
		m_skipList.UpdateNode(iter->second, id, slNewKey);
		slNewKey.CopyTo(iter->second);
	} else {
		CSkipListKey slKey(score);
		m_timeSeries.Generate(slKey.m_u32Time, slKey.m_u32Count);
		util::CAutoPointer<IRankData<ScoreType> > rankData(new RankDataType);
		slKey.CopyTo(rankData);
		m_id2RankData.insert(iter, typename ID_TO_RANKDAT_T::value_type(id, rankData));
		m_skipList.AddNode(slKey, id);
		outAddData = rankData;
		if(0 != m_uLimitSize && m_skipList.Size() > m_uLimitSize) {
			typename SKIP_LIST_T::RIterator iter(m_skipList.RBegin());
			if(m_skipList.REnd() != iter) {
				DelItem(outRemoveData, iter->GetValue());
			}
		}
	}
	return true;
}

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::DelItem(IDType id) {
	thd::CScopedWriteLock wrLock(m_rwLock);
	typename ID_TO_RANKDAT_T::const_iterator iter(m_id2RankData.find(id));
	if(m_id2RankData.end() == iter) {
		return false;
	}
	CSkipListKey slKey(iter->second);
	m_skipList.DeleteNode(slKey, id);
	m_id2RankData.erase(iter);
	return true;
}

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::DelItem(util::CAutoPointer<RankDataType>& outRankData, IDType id) {
	thd::CScopedWriteLock wrLock(m_rwLock);
	typename ID_TO_RANKDAT_T::const_iterator iter(m_id2RankData.find(id));
	if(m_id2RankData.end() == iter) {
		return false;
	}
	outRankData = iter->second;
	CSkipListKey slKey(iter->second);
	m_skipList.DeleteNode(slKey, id);
	m_id2RankData.erase(iter);
	return true;
}

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::GetItem(util::CAutoPointer<RankDataType>& outRankData, IDType id) {
	thd::CScopedReadLock rdLock(m_rwLock);
	typename ID_TO_RANKDAT_T::const_iterator iter(m_id2RankData.find(id));
	if(m_id2RankData.end() == iter) {
		return false;
	}
	outRankData = iter->second;
	return true;
}

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::GetRankAndScore(unsigned long& outRank, ScoreType& outScore, IDType id) {
	thd::CScopedReadLock rdLock(m_rwLock);
	typename ID_TO_RANKDAT_T::const_iterator iter(m_id2RankData.find(id));
	if(m_id2RankData.end() == iter) {
		return false;
	}

	CSkipListKey slKey(iter->second);
	outRank = m_skipList.GetRank(slKey, id);
	outScore = iter->second->GetScore();
	return true;
}

template <class IDType, class ScoreType, class RankDataType>
unsigned long CRankSetMT<IDType, ScoreType, RankDataType>::GetRank(IDType id) {
	thd::CScopedReadLock rdLock(m_rwLock);
	typename ID_TO_RANKDAT_T::const_iterator iter(m_id2RankData.find(id));
	if(m_id2RankData.end() == iter) {
		return 0;
	}
	CSkipListKey slKey(iter->second);
	return m_skipList.GetRank(slKey, id);
}

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::GetTopRanks(NodeRankType* outNodes, uint32_t u32Count) {
	if(NULL == outNodes) {
		return false;
	}

	if(0 == u32Count) {
		return false;
	}

	thd::CScopedReadLock rdLock(m_rwLock);

	typename SKIP_LIST_T::Iterator iter(m_skipList.Begin());
	for(uint32_t i = 0; i < u32Count && iter != m_skipList.End(); ++i,++iter) {

		outNodes[i].pNode = iter.operator ->();
		outNodes[i].uRank = i + 1;
	}
	return true;
}

template <class IDType, class ScoreType, class RankDataType>
unsigned long CRankSetMT<IDType, ScoreType, RankDataType>::GetSize()const {
	thd::CScopedReadLock rdLock(m_rwLock);
	return m_skipList.Size();
}

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::GetFirstMoreNodes(NodeRankType* outNodes, uint32_t u32Count, IDType id) {
	thd::CScopedReadLock rdLock(m_rwLock);
	typename ID_TO_RANKDAT_T::const_iterator iter(m_id2RankData.find(id));
	if(m_id2RankData.end() == iter) {
		return false;
	}
	CSkipListKey slKey(iter->second);
	return m_skipList.GetFirstMoreNodes(outNodes, u32Count, slKey, id);
}

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::GetLastMoreNodes(NodeRankType* outNodes, uint32_t u32Count, IDType id) {
	thd::CScopedReadLock rdLock(m_rwLock);
	typename ID_TO_RANKDAT_T::const_iterator iter(m_id2RankData.find(id));
	if(m_id2RankData.end() == iter) {
		return false;
	}
	CSkipListKey slKey(iter->second);
	return m_skipList.GetLastMoreNodes(outNodes, u32Count, slKey, id);
}

template <class IDType, class ScoreType, class RankDataType>
bool CRankSetMT<IDType, ScoreType, RankDataType>::GetMidMoreNodes(NodeRankType* outNodes, uint32_t u32Count, IDType id, bool reverse) {
	thd::CScopedReadLock rdLock(m_rwLock);
	typename ID_TO_RANKDAT_T::const_iterator iter(m_id2RankData.find(id));
	if(m_id2RankData.end() == iter) {
		return false;
	}
	CSkipListKey slKey(iter->second);
	return m_skipList.GetMidMoreNodes(outNodes, u32Count, slKey, id, reverse);
}

}

#endif /* __RANKSETMT_H_ */
