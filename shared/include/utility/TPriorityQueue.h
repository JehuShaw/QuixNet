/*
 * File:   TPriorityQueue.h
 * Author: Jehu Shaw
 *
 * Created on 2014_5_8, 14:09
 */

#ifndef TPRIORITYQUEUE_H
#define TPRIORITYQUEUE_H

#include "ITPriorityQueue.h"
#include "ITComparer.h"
#include <vector>
#include <assert.h>

namespace util {

class CTPriorityQueue : public ITPriorityQueue
{
public:
    typedef std::vector<CAutoPointer<IPQElementBase> > HEAP_T;

public:
    /**
     * construction                                                                   
     */
    CTPriorityQueue(CAutoPointer<ITComparer> comparer)
	: m_pComparer(comparer) {
		assert(!m_pComparer.IsInvalid());
	}

    CTPriorityQueue(const CAutoPointer<ITComparer>& comparer, int nSize)
	: m_pComparer(comparer) {
		assert(!m_pComparer.IsInvalid());
		m_heap.reserve(nSize);
	}
    /**
     * disconstruction                                                                     
     */
    ~CTPriorityQueue(void) {

	}
    /**
     * Push an element to the queue
     * return A position of the element
     */
    void Push(CAutoPointer<IPQElementBase> element) {
		int p = m_heap.size(),p2;
		m_heap.push_back(element); // E[p] = O
		do
		{
			if(0 == p) {
				break;
			}
			p2 = (p-1)/2;
			if(OnCompare(p,p2)<0) {
				SwitchElements(p,p2);
				p = p2;
			}
			else {
				break;
			}
		} while(true);
		SetElementIdx(m_heap[p], p);
	}
    /**
     * Pop the top element                                                                     
     */
    CAutoPointer<IPQElementBase> Pop() {

		CAutoPointer<IPQElementBase> result = m_heap[0];
		int p = 0,p1,p2,pn;
		m_heap[0] = m_heap[m_heap.size()-1];
		SetElementIdx(m_heap[0], 0);
		m_heap.pop_back();
		do
		{
			pn = p;
			p1 = 2*p+1;
			p2 = 2*p+2;
			if((int)m_heap.size() > p1 && OnCompare(p,p1) > 0) {
				// links kleiner
				p = p1;
			}
			if((int)m_heap.size() > p2 && OnCompare(p,p2) > 0) {
				// rechts noch kleiner
				p = p2;
			}
			if(p == pn) {
				break;
			}
			SwitchElements(p,pn);
		}while(true);

		return result;
	}
    /**
     * Retrieve the top element and don't remove it                                                                 
     */
    CAutoPointer<IPQElementBase> Peek() const {
		if(!m_heap.empty()) {
			return m_heap[0];
		}
		return CAutoPointer<IPQElementBase>();
	}
    /**
     * Update the element by position.                                                                
     */
    void Update(int nIdx) {

		if(nIdx >= (int)m_heap.size() || nIdx < 0) {
			assert(false);
			return;
		}

		int p = nIdx,pn;
		int p1,p2;
		do	// aufsteigen
		{
			if(0 == p) {
				break;
			}
			p2 = (p - 1)/2;
			if(OnCompare(p,p2) < 0) {
				SwitchElements(p, p2);
				p = p2;
			}
			else {
				break;
			}
		}while(true);
		if(p < nIdx) {
			return;
		}
		do	   // absteigen
		{
			pn = p;
			p1 = 2*p+1;
			p2 = 2*p+2;
			if((int)m_heap.size() > p1 && OnCompare(p, p1) > 0) {
				// links kleiner
				p = p1;
			}
			if((int)m_heap.size() > p2 && OnCompare(p, p2) > 0) {
				// rechts noch kleiner
				p = p2;
			}
			if(p == pn) {
				break;
			}
			SwitchElements(p, pn);
		} while(true);
	}
	inline void Update(CAutoPointer<IPQElementBase> element) {
		if(element.IsInvalid()) {
			return;
		}
		Update((int)element->GetIndex());
	}
    /**
     * Check the queue                                                                      
     */
    bool IsEmpty() const {
		return m_heap.empty();
	}
    /**
     * Clear the queue                                                                     
     */
    void Clear() {
		m_heap.clear();
	}
    /**
     * The size of the queue                                                                     
     */
    int Count() const {
		return (int)m_heap.size();
	}
    /**
     * Remove the element                                                                     
     */
    void Remove(int nIdx) {

		if(m_heap.empty()) {
			return;
		}

		if(nIdx < 0 || (int)m_heap.size() <= nIdx) {
			assert(false);
			return;
		}

		if(0 != nIdx) {
			m_heap[nIdx] = Peek();
			SetElementIdx(m_heap[nIdx], nIdx);
			Update(nIdx);
		}
		Pop();
	}
	inline void Remove(CAutoPointer<IPQElementBase> element) {
		if(element.IsInvalid()) {
			return;
		}
		Remove((int)element->GetIndex());
	}

protected:

    inline void SwitchElements( int i, int j ) {
        CAutoPointer<IPQElementBase> h = m_heap[i];
        m_heap[i] = m_heap[j];
        m_heap[j] = h;
		SetElementIdx(m_heap[i], i);
		SetElementIdx(m_heap[j], j);
    }

    inline int OnCompare( int i, int j ) const {
        return m_pComparer->Compare(m_heap[i], m_heap[j]);
    }

protected:
    HEAP_T m_heap;
    CAutoPointer<ITComparer> m_pComparer;
};

}

#endif /* TPRIORITYQUEUE_H */