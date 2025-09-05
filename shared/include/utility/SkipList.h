
#ifndef SKIPLIST_RANK_H
#define SKIPLIST_RANK_H

#include<stdlib.h>

namespace util {

const int MAX_LEV_SIZE = 8;

template<class KeyType, class ValueType>
class CSkipListNode
{
public:
	CSkipListNode(const KeyType& k, const ValueType& v) : m_key(k), m_value(v), m_pBackward(NULL) {}
	CSkipListNode() : m_key(), m_value(), m_pBackward(NULL) {}

	KeyType GetKey()const { return m_key; }

	ValueType GetValue()const { return m_value; }

private:
	template<class FKeyType, class FValueType> friend class CSkipList;
	template<class FKeyType, class FValueType> friend class CSkipListIterator;
	template<class FKeyType, class FValueType> friend class CSkipListRIterator;

	KeyType m_key;
	ValueType m_value;
	CSkipListNode* m_pBackward;

	struct SkipListLev {
		CSkipListNode* m_pForward;
		unsigned int m_nSpan;
	}m_lev[1];
};

template<class KeyType, class ValueType>
class CSkipListIterator
{
public:
	typedef CSkipListNode<KeyType, ValueType> NodeType;

	CSkipListIterator(NodeType* pNode) : m_pNode(pNode) {}
	CSkipListIterator() : m_pNode(NULL) {}

	NodeType& operator*() {
		return *m_pNode;
	}

	NodeType* operator ->() {
		return m_pNode;
	}

	void operator++() {
		m_pNode = m_pNode->m_lev[0].m_pForward;
	}

	bool operator!= (const CSkipListIterator& right) const {
		return this->m_pNode != right.m_pNode;
	}

private:
	NodeType* m_pNode;
};

template<class KeyType, class ValueType>
class CSkipListRIterator
{
public:
	typedef CSkipListNode<KeyType, ValueType> NodeType;

	CSkipListRIterator(NodeType* pNode) : m_pNode(pNode) {}
	CSkipListRIterator() : m_pNode(NULL) {}

	NodeType& operator*() {
		return *m_pNode;
	}

	NodeType* operator ->() {
		return m_pNode;
	}

	void operator++() {
		m_pNode = m_pNode->m_pBackward;
	}

	bool operator!= (const CSkipListRIterator& right) const {
		return this->m_pNode != right.m_pNode;
	}

private:
	NodeType* m_pNode;
};

template<class KeyType, class ValueType>
class CSkipList
{
public:
	typedef CSkipListNode<KeyType, ValueType> NodeType;
	typedef CSkipListIterator<KeyType, ValueType> Iterator;
	typedef CSkipListRIterator<KeyType, ValueType> RIterator;
	typedef struct stNodeRankType {
		NodeType* pNode;
		unsigned long uRank;
		stNodeRankType() : pNode(NULL), uRank(0) {}
	} NodeRankType;

	CSkipList();
	~CSkipList();

public:
	/* Insert a new node in the skip list. Assumes the element does not already
	 * exist (up to the caller to enforce that).
	 */
	bool AddNode(const KeyType& key, const ValueType& value);
	/* Delete an element with matching key/element from the skip list.
	 * The function returns true if the node was found and deleted, otherwise
	 * false is returned.
	 *
	 * The deleted node is freed by delete operator.
	 */
	bool DeleteNode(const KeyType& key, const ValueType& value);
	/* Update an element with matching old key/element from the skip list
	 * and change the old key into new key value.
	 * The function returns true if the node was found and deleted, otherwise
	 * false is returned.
	 */
	bool UpdateNode(const KeyType& oldKey, const ValueType& value, const KeyType& newKey);
	/* Find the rank for an element by both key and value.
	 * Returns 0 when the element cannot be found, rank otherwise.
	 * Note that the rank is 1-based due to the span of m_pHeader to the
	 * first element.
	 */
	unsigned long GetRank(const KeyType& key, const ValueType& value);
	/* Finds an element by its rank. The rank argument needs to be 1-based.
	 */
	NodeType* GetNodeByRank(unsigned long rank);

	bool GetFirstMoreNodesByRank(NodeRankType* outNodes, unsigned long count, unsigned long firstRank);

	bool GetLastMoreNodesByRank(NodeRankType* outNodes, unsigned long count, unsigned long lastRank);

	bool GetMidMoreNodesByRank(NodeRankType* outNodes, unsigned long count, unsigned long midRank, bool reverse = false);

	bool GetFirstMoreNodes(NodeRankType* outNodes, unsigned long count, const KeyType& key, const ValueType& value);

	bool GetLastMoreNodes(NodeRankType* outNodes, unsigned long count, const KeyType& key, const ValueType& value);

	bool GetMidMoreNodes(NodeRankType* outNodes, unsigned long count, const KeyType& key, const ValueType& value, bool reverse = false);
	/* Get the size of skip list.
	 */
	unsigned long Size() const { return m_ulLength; }

public:
	Iterator Begin() {
		if(NULL == m_pHeader) {
			return Iterator();
		}

		return Iterator(m_pHeader->m_lev[0].m_pForward);
	}

	Iterator End() {
		return Iterator();
	}

	RIterator RBegin() {
		if(NULL == m_pTail) {
			return RIterator();
		}

		return RIterator(m_pTail);
	}

	RIterator REnd() {
		return RIterator();
	}

private:
	void Free();
	void Init();

private:
	NodeType* CreateNode(int lev);
	NodeType* CreateNode(int lev, const KeyType& key, const ValueType& value);
	int DeleteNode(NodeType* node, NodeType** update);
	int RandomLevel();

private:
	int m_nCurrentLev;
	unsigned long m_ulLength;
	NodeType *m_pHeader, *m_pTail;

};

template<class KeyType , class ValueType>
CSkipList<KeyType, ValueType>::CSkipList()
	: m_nCurrentLev(0)
	, m_ulLength(0)
	, m_pHeader(NULL)
	, m_pTail(NULL)
{
	Init();
}

template<class KeyType , class ValueType>
CSkipList<KeyType, ValueType>::~CSkipList() {
	Free();
}

template<class KeyType , class ValueType>
void CSkipList<KeyType, ValueType>::Init() {
	m_nCurrentLev = 1;
	m_ulLength = 0;

	m_pHeader = CreateNode(MAX_LEV_SIZE);
	for(int i = 0; i < MAX_LEV_SIZE; ++i) {
		m_pHeader->m_lev[i].m_pForward = NULL;
		m_pHeader->m_lev[i].m_nSpan = 0;
	}

	m_pHeader->m_pBackward = NULL;
}


template<class KeyType , class ValueType>
void CSkipList<KeyType, ValueType>::Free() {
	if(m_pHeader == NULL) {
		return;
	}

	NodeType *pNode = m_pHeader->m_lev[0].m_pForward, *pNext;

	while(pNode) {
		pNext = pNode->m_lev[0].m_pForward;
		delete pNode;
		pNode = pNext;
	}

	delete m_pHeader;
	m_pHeader = NULL;
}

template<class KeyType , class ValueType>
bool CSkipList<KeyType, ValueType>::AddNode(const KeyType& key, const ValueType& value) {
	if(NULL == m_pHeader) {
		return false;
	}

	NodeType *update[MAX_LEV_SIZE], *x;
	unsigned int rank[MAX_LEV_SIZE];
	int i, level;
	x = m_pHeader;

	for(i = m_nCurrentLev - 1; i >= 0; --i) {
        rank[i] = i == (m_nCurrentLev - 1) ? 0 : rank[i+1];
        while(x->m_lev[i].m_pForward &&
            (x->m_lev[i].m_pForward->m_key < key ||
                (x->m_lev[i].m_pForward->m_key == key &&
				x->m_lev[i].m_pForward->m_value < value))) {
            rank[i] += x->m_lev[i].m_nSpan;
            x = x->m_lev[i].m_pForward;
        }

        update[i] = x;
	}

	level = RandomLevel();

	if(level > m_nCurrentLev) {
		for(i = m_nCurrentLev; i < level; ++i) {
			rank[i] = 0;
            update[i] = m_pHeader;
            update[i]->m_lev[i].m_nSpan = m_ulLength;
		}

		m_nCurrentLev = level;
	}

	x = CreateNode(level, key, value);
	for(i = 0; i < level; ++i) {
        x->m_lev[i].m_pForward = update[i]->m_lev[i].m_pForward;
        update[i]->m_lev[i].m_pForward = x;

        /* update span covered by update[i] as x is inserted here */
        x->m_lev[i].m_nSpan = update[i]->m_lev[i].m_nSpan - (rank[0] - rank[i]);
        update[i]->m_lev[i].m_nSpan = (rank[0] - rank[i]) + 1;
    }

    /* increment span for untouched levels */
    for(i = level; i < m_nCurrentLev; ++i) {
        ++(update[i]->m_lev[i].m_nSpan);
    }

    x->m_pBackward = (update[0] == m_pHeader) ? NULL : update[0];
    if(x->m_lev[0].m_pForward) {
        x->m_lev[0].m_pForward->m_pBackward = x;
	} else {
        m_pTail = x;
	}

	++m_ulLength;

	return true;
}

template<class KeyType , class ValueType>
int CSkipList<KeyType, ValueType>::RandomLevel() {
	int nLevel = 1;
	while ((::rand()&0xFFFF) < (0.25 * 0xFFFF)) {
        ++nLevel;
	}

    return (nLevel<MAX_LEV_SIZE) ? nLevel : MAX_LEV_SIZE;
}

template<class KeyType, class ValueType>
typename CSkipList<KeyType, ValueType>::NodeType* CSkipList<KeyType, ValueType>::CreateNode(int lev) {
	if(lev < 1) {
		lev = 1;
	}
	void * pBytes = malloc(sizeof(NodeType) + lev * sizeof(typename NodeType::SkipListLev));
	NodeType* pNode = new(pBytes) NodeType;

	return pNode;
}

template<class KeyType, class ValueType>
typename CSkipList<KeyType, ValueType>::NodeType* CSkipList<KeyType, ValueType>::CreateNode(int lev, const KeyType& key, const ValueType& value) {
	if(lev < 1) {
		lev = 1;
	}
	void * pBytes = malloc(sizeof(NodeType) + lev * sizeof(typename NodeType::SkipListLev));
	NodeType* pNode = new(pBytes) NodeType(key, value);

	return pNode;
}

template<class KeyType, class ValueType>
unsigned long CSkipList<KeyType, ValueType>::GetRank(const KeyType& key, const ValueType& value) {
    NodeType *x;
    unsigned long rank = 0;
    int i;

	x = m_pHeader;
	for(i = m_nCurrentLev - 1; i >= 0; --i) {
		while(x->m_lev[i].m_pForward &&
            (x->m_lev[i].m_pForward->m_key < key ||
                (x->m_lev[i].m_pForward->m_key == key &&
                x->m_lev[i].m_pForward->m_value <= value))) {
            rank += x->m_lev[i].m_nSpan;
            x = x->m_lev[i].m_pForward;
        }

        /* x might be equal to m_pHeader, so test if obj is non-NULL */
        if(x->m_value == value) {
            return rank;
        }
    }
    return 0;
}

template<class KeyType, class ValueType>
bool CSkipList<KeyType, ValueType>::DeleteNode(const KeyType& key, const ValueType& value) {
	NodeType *update[MAX_LEV_SIZE], *x;
    int i;

	// find
    x = m_pHeader;
	for(i = m_nCurrentLev - 1; i >= 0; --i) {
        while (x->m_lev[i].m_pForward &&
            (x->m_lev[i].m_pForward->m_key < key ||
                (x->m_lev[i].m_pForward->m_key == key &&
                x->m_lev[i].m_pForward->m_value < value))) {
            x =  x->m_lev[i].m_pForward;
		}

        update[i] = x;
    }
    /* We may have multiple elements with the same score, what we need
     * is to find the element with both the right score and object. */
	x = x->m_lev[0].m_pForward;
	if(x && key == x->m_key && x->m_value == value) {
		DeleteNode(x, update);
		delete x;
		return true;
	}

    return false;
}

template<class KeyType, class ValueType>
int CSkipList<KeyType, ValueType>::DeleteNode(NodeType* x, NodeType** update) {
    int lev = 0;
	for(int i = 0; i < m_nCurrentLev; i++) {
        if (update[i]->m_lev[i].m_pForward == x) {
			++lev;
            update[i]->m_lev[i].m_nSpan += x->m_lev[i].m_nSpan - 1;
            update[i]->m_lev[i].m_pForward = x->m_lev[i].m_pForward;
        } else {
            update[i]->m_lev[i].m_nSpan -= 1;
        }
    }
    if(x->m_lev[0].m_pForward) {
        x->m_lev[0].m_pForward->m_pBackward = x->m_pBackward;
    } else {
		this->m_pTail = x->m_pBackward;
    }
    while(m_nCurrentLev > 1 && m_pHeader->m_lev[m_nCurrentLev - 1].m_pForward == NULL) {
        --m_nCurrentLev;
	}

	--m_ulLength;

	return lev;
}

template<class KeyType, class ValueType>
bool CSkipList<KeyType, ValueType>::UpdateNode(const KeyType& oldKey, const ValueType& oldValue, const KeyType& newKey) {
	if(NULL == m_pHeader) {
		return false;
	}

	if(oldKey == newKey) {
		return true;
	}

	NodeType *update[MAX_LEV_SIZE], *x, *f;
	unsigned int rank[MAX_LEV_SIZE];
	int i, level;

	// find old key node
	x = m_pHeader;
	for(i = m_nCurrentLev - 1; i >= 0; --i) {
		while(x->m_lev[i].m_pForward &&
			(x->m_lev[i].m_pForward->m_key < oldKey ||
			(x->m_lev[i].m_pForward->m_key == oldKey &&
			x->m_lev[i].m_pForward->m_value < oldValue))) {
				x = x->m_lev[i].m_pForward;
		}
		update[i] = x;
	}

	x = x->m_lev[0].m_pForward;
	if(x && x->m_key == oldKey && x->m_value == oldValue) {
		level = DeleteNode(x, update);
		x->m_key = newKey;
		x->m_pBackward = NULL;
	} else {
		return false;
	}

	// find new key position
	f = m_pHeader;
	for(i = m_nCurrentLev - 1; i >= 0; --i) {
		rank[i] = i == (m_nCurrentLev - 1) ? 0 : rank[i+1];
		while(f->m_lev[i].m_pForward &&
			(f->m_lev[i].m_pForward->m_key < newKey ||
			(f->m_lev[i].m_pForward->m_key == newKey &&
			f->m_lev[i].m_pForward->m_value < oldValue))) {
				rank[i] += f->m_lev[i].m_nSpan;
				f = f->m_lev[i].m_pForward;
		}

		update[i] = f;
	}

	for(i = 0; i < level; ++i) {
		x->m_lev[i].m_pForward = update[i]->m_lev[i].m_pForward;
		update[i]->m_lev[i].m_pForward = x;

		/* update span covered by update[i] as x is inserted here */
		x->m_lev[i].m_nSpan = update[i]->m_lev[i].m_nSpan - (rank[0] - rank[i]);
		update[i]->m_lev[i].m_nSpan = (rank[0] - rank[i]) + 1;
	}

	/* increment span for untouched levels */
	for(i = level; i < m_nCurrentLev; ++i) {
		++(update[i]->m_lev[i].m_nSpan);
	}

	x->m_pBackward = (update[0] == m_pHeader) ? NULL : update[0];
	if(x->m_lev[0].m_pForward) {
		x->m_lev[0].m_pForward->m_pBackward = x;
	} else {
		m_pTail = x;
	}

	++m_ulLength;

	return true;
}

template<class KeyType, class ValueType>
typename CSkipList<KeyType, ValueType>::NodeType* CSkipList<KeyType, ValueType>::GetNodeByRank(unsigned long rank) {
	if(0 == rank) {
		return NULL;
	}

	NodeType *x;
	unsigned long traversed = 0;
	int i;

	x = m_pHeader;
	for (i = m_nCurrentLev - 1; i >= 0; --i) {
		while (x->m_lev[i].m_pForward && (traversed + x->m_lev[i].m_nSpan) <= rank) {
			traversed += x->m_lev[i].m_nSpan;
			x = x->m_lev[i].m_pForward;
		}
		if(traversed == rank) {
			return x;
		}
	}

	return NULL;
}

template<class KeyType, class ValueType>
bool CSkipList<KeyType, ValueType>::GetFirstMoreNodesByRank(NodeRankType* outNodes, unsigned long count, unsigned long firstRank)
{
	if(NULL == outNodes) {
		return false;
	}

	if(0 == firstRank || 0 == count) {
		return false;
	}

	NodeType *x;
	unsigned long traversed = 0;
	int i;

	x = m_pHeader;
	for (i = m_nCurrentLev - 1; i >= 0; --i) {
		while (x->m_lev[i].m_pForward && (traversed + x->m_lev[i].m_nSpan) <= firstRank) {
			traversed += x->m_lev[i].m_nSpan;
			x = x->m_lev[i].m_pForward;
		}
		if(traversed == firstRank) {

			long long leave = m_ulLength - firstRank + 1;
			leave = leave < (long long)count ? leave : count;
			for(unsigned long j = 0; j < leave; ++j) {
				outNodes[j].pNode = x;
				outNodes[j].uRank = firstRank + j;
				x = x->m_lev[0].m_pForward;
			}
			return true;
		}
	}
	return false;
}

template<class KeyType, class ValueType>
bool CSkipList<KeyType, ValueType>::GetLastMoreNodesByRank(NodeRankType* outNodes, unsigned long count, unsigned long lastRank)
{
	if(NULL == outNodes) {
		return false;
	}

	if(0 == lastRank || 0 == count) {
		return false;
	}

	NodeType *x;
	unsigned long traversed = 0;
	int i;

	x = m_pHeader;
	for (i = m_nCurrentLev - 1; i >= 0; --i) {
		while (x->m_lev[i].m_pForward && (traversed + x->m_lev[i].m_nSpan) <= lastRank) {
			traversed += x->m_lev[i].m_nSpan;
			x = x->m_lev[i].m_pForward;
		}
		if(traversed == lastRank) {

			unsigned long leave = lastRank < count ? lastRank : count;
			for(unsigned long j = 0; j < leave; ++j) {
				outNodes[j].pNode = x;
				outNodes[j].uRank = lastRank - j;
				x = x->m_pBackward;
			}
			return true;
		}
	}
	return false;
}

template<class KeyType, class ValueType>
bool CSkipList<KeyType, ValueType>::GetMidMoreNodesByRank(NodeRankType* outNodes, unsigned long count, unsigned long midRank, bool reverse) {
	if(NULL == outNodes) {
		return false;
	}

	if(0 == midRank || 0 == count) {
		return false;
	}

	NodeType *x;
	unsigned long traversed = 0;
	int i;

	x = m_pHeader;
	for (i = m_nCurrentLev - 1; i >= 0; --i) {
		while (x->m_lev[i].m_pForward && (traversed + x->m_lev[i].m_nSpan) <= midRank) {
			traversed += x->m_lev[i].m_nSpan;
			x = x->m_lev[i].m_pForward;
		}
		if(traversed == midRank) {

			unsigned long mid = count / 2 + 1;

			if(reverse) {

				for(long j = mid - 1; j > -1; --j) {
					outNodes[j].pNode = x;
					outNodes[j].uRank = midRank + mid - 1 - j;

					x = x->m_lev[0].m_pForward;
					if(NULL == x) {
						break;
					}
				}

				x = outNodes[mid - 1].pNode;
				for(unsigned long j = mid; j < count; ++j) {
					x = x->m_pBackward;
					if(NULL == x) {
						break;
					}

					outNodes[j].pNode = x;
					outNodes[j].uRank = midRank + mid - 1 - j;
				}
			} else {

				for(long j = mid - 1; j > -1; --j) {
					outNodes[j].pNode = x;
					outNodes[j].uRank = midRank - mid + 1 + j;

					x = x->m_pBackward;
					if(NULL == x) {
						break;
					}
				}

				x = outNodes[mid - 1].pNode;
				for(unsigned long j = mid; j < count; ++j) {
					x = x->m_lev[0].m_pForward;
					if(NULL == x) {
						break;
					}

					outNodes[j].pNode = x;
					outNodes[j].uRank = midRank - mid + 1 + j;
				}
			}
			return true;
		}
	}
	return false;
}

template<class KeyType, class ValueType>
bool CSkipList<KeyType, ValueType>::GetFirstMoreNodes(NodeRankType* outNodes, unsigned long count, const KeyType& key, const ValueType& value) {
	if(NULL == outNodes) {
		return false;
	}

	if(0 == count) {
		return false;
	}

	NodeType *x;
	unsigned long rank = 0;
	int i;

	x = m_pHeader;
	for(i = m_nCurrentLev - 1; i >= 0; --i) {
		while(x->m_lev[i].m_pForward &&
			(x->m_lev[i].m_pForward->m_key < key ||
			(x->m_lev[i].m_pForward->m_key == key &&
			x->m_lev[i].m_pForward->m_value <= value))) {
				rank += x->m_lev[i].m_nSpan;
				x = x->m_lev[i].m_pForward;
		}

		/* x might be equal to m_pHeader, so test if obj is non-NULL */
		if(x->m_value == value) {
			long long leave = m_ulLength - rank + 1;
			leave = leave < (long long)count ? leave : count;
			for(unsigned long j = 0; j < leave; ++j) {
				outNodes[j].pNode = x;
				outNodes[j].uRank = rank + j;
				x = x->m_lev[0].m_pForward;
			}
			return true;
		}
	}
	return false;
}

template<class KeyType, class ValueType>
bool CSkipList<KeyType, ValueType>::GetLastMoreNodes(NodeRankType* outNodes, unsigned long count, const KeyType& key, const ValueType& value) {
	if(NULL == outNodes) {
		return false;
	}

	if(0 == count) {
		return false;
	}

	NodeType *x;
	unsigned long rank = 0;
	int i;

	x = m_pHeader;
	for(i = m_nCurrentLev - 1; i >= 0; --i) {
		while(x->m_lev[i].m_pForward &&
			(x->m_lev[i].m_pForward->m_key < key ||
			(x->m_lev[i].m_pForward->m_key == key &&
			x->m_lev[i].m_pForward->m_value <= value))) {
				rank += x->m_lev[i].m_nSpan;
				x = x->m_lev[i].m_pForward;
		}

		/* x might be equal to m_pHeader, so test if obj is non-NULL */
		if(x->m_value == value) {
			unsigned long leave = rank < count ? rank : count;
			for(unsigned long j = 0; j < leave; ++j) {
				outNodes[j].pNode = x;
				outNodes[j].uRank = rank - j;
				x = x->m_pBackward;
			}
			return true;
		}
	}
	return false;
}

template<class KeyType, class ValueType>
bool CSkipList<KeyType, ValueType>::GetMidMoreNodes(NodeRankType* outNodes, unsigned long count, const KeyType& key, const ValueType& value, bool reverse/* = false*/) {
	if(NULL == outNodes) {
		return false;
	}

	if(0 == count) {
		return false;
	}

	NodeType *x;
	unsigned long rank = 0;
	int i;

	x = m_pHeader;
	for(i = m_nCurrentLev - 1; i >= 0; --i) {
		while(x->m_lev[i].m_pForward &&
			(x->m_lev[i].m_pForward->m_key < key ||
			(x->m_lev[i].m_pForward->m_key == key &&
			x->m_lev[i].m_pForward->m_value <= value))) {
				rank += x->m_lev[i].m_nSpan;
				x = x->m_lev[i].m_pForward;
		}

		/* x might be equal to m_pHeader, so test if obj is non-NULL */
		if(x->m_value == value) {

			unsigned long mid = count / 2 + 1;

			if(reverse) {

				for(long j = mid - 1; j > -1; --j) {
					outNodes[j].pNode = x;
					outNodes[j].uRank = rank + mid - 1 - j;

					x = x->m_lev[0].m_pForward;
					if(NULL == x) {
						break;
					}
				}

				x = outNodes[mid - 1].pNode;
				for(unsigned long j = mid; j < count; ++j) {
					x = x->m_pBackward;
					if(NULL == x) {
						break;
					}

					outNodes[j].pNode = x;
					outNodes[j].uRank = rank + mid - 1 - j;
				}
			} else {

				for(long j = mid - 1; j > -1; --j) {
					outNodes[j].pNode = x;
					outNodes[j].uRank = rank - mid + 1 + j;

					x = x->m_pBackward;
					if(NULL == x) {
						break;
					}
				}

				x = outNodes[mid - 1].pNode;
				for(unsigned long j = mid; j < count; ++j) {
					x = x->m_lev[0].m_pForward;
					if(NULL == x) {
						break;
					}

					outNodes[j].pNode = x;
					outNodes[j].uRank = rank - mid + 1 + j;
				}
			}
			return true;
		}
	}
	return false;
}

}

#endif /* SKIPLIST_RANK_H */
