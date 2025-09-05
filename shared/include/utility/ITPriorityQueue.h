/*
 * File:   ITPriorityQueue.h
 * Author: Jehu Shaw
 *
 * Created on 2014_5_8, 14:09
 */

#ifndef ITPRIORITYQUEUE_H
#define ITPRIORITYQUEUE_H

#include "AutoPointer.h"

namespace util {

class IPQElementBase {
public:
	virtual ~IPQElementBase() {}
	virtual int GetIndex() const = 0;

protected:
	virtual void SetIndex(int nIdx) = 0;
	friend class ITPriorityQueue;
};

class ITPriorityQueue
{
public:
    virtual ~ITPriorityQueue() {}
    /// <summary>
    /// Push an object onto the PQ
    /// </summary>
    /// <param name="element">The new object</param>
    /// This will change when objects are taken from or put onto the PQ.</returns>
    virtual void Push(CAutoPointer<IPQElementBase> element) = 0;
    /// <summary>
    /// Get the smallest object and remove it.
    /// </summary>
    /// <returns>The smallest object</returns>
    virtual CAutoPointer<IPQElementBase> Pop() = 0;
    /// <summary>
    /// Get the smallest object without removing it.
    /// </summary>
    /// <returns>The smallest object</returns>
    virtual CAutoPointer<IPQElementBase> Peek() const = 0;
    /// <summary>
    /// Notify the PQ that the object at position i has changed
    /// and the PQ needs to restore order.
    /// Since you dont have access to any indexes (except by using the
    /// explicit IList.this) you should not call this function without knowing exactly
    /// what you do.
    /// </summary>
    /// <param name="nIdx">The index of the changed object.</param>
    virtual void Update(int nIdx) = 0;
    /// <summary>
    /// Check the PQ is Empty size;
    /// </summary>
    /// <returns>boolean</returns>
    virtual bool IsEmpty() const = 0;
    /// <summary>
    /// Clear the PQ;
    /// </summary>
    /// <returns>boolean</returns>
    virtual void Clear() = 0;
    /// <summary>
    /// Get the PQ element size
    /// </summary>
    /// <returns>boolean</returns>
    virtual int Count() const = 0;
    /// <summary>
    /// Notify the PQ that the object at position i want be removed
    /// </summary>
    /// <param name="nIdx">The index of the changed object.</param>
    virtual void Remove(int nIdx) = 0;

protected:
	/// <summary>
	/// Set the index of element
	/// </summary>
	/// <param name="nIdx">The index of the changed object.</param>
	inline static void SetElementIdx(CAutoPointer<IPQElementBase>& element, int nIdx) {
		if(element.IsInvalid()) {
			return;
		}
		element->SetIndex(nIdx);
	}
};

}

#endif /* ITPRIORITYQUEUE_H */