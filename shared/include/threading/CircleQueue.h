
#ifndef __CIRCLE_QUEUE_H
#define __CIRCLE_QUEUE_H

#include <assert.h>
#include "SpinLock.h"

namespace thd {
/// \brief A single producer consumer implementation without critical sections.
template <class ElementType>
class CCircleQueue
{
public:
    /// Constructor
    CCircleQueue(unsigned long minimunSize = 8)  throw();

    /// Destructor
    ~CCircleQueue()  throw();

    /// WriteLock must be immediately followed by WriteUnlock.  These two functions must be called in the same thread.
    /// \return A pointer to a block of data you can write to.
    ElementType* WriteLock(void)  throw();

    /// Call if you don't want to write to a block of data from WriteLock() after all.
    /// Cancelling locks cancels all locks back up to the data passed.  So if you lock twice and cancel using the first lock, the second lock is ignored
    /// \param[in] cancelToLocation Which WriteLock() to cancel.
    void CancelWriteLock(ElementType* cancelToLocation)  throw();

    /// Call when you are done writing to a block of memory returned by WriteLock()
    void WriteUnlock(void)  throw();

    /// ReadLock must be immediately followed by ReadUnlock. These two functions must be called in the same thread.
    /// \retval 0 No data is availble to read
    /// \retval Non-zero The data previously written to, in another thread, by WriteLock followed by WriteUnlock.
    ElementType* ReadLock(void)  throw();

    // Cancelling locks cancels all locks back up to the data passed.  So if you lock twice and cancel using the first lock, the second lock is ignored
    /// param[in] Which ReadLock() to cancel.
    void CancelReadLock(ElementType* cancelToLocation)  throw();

    /// Signals that we are done reading the the data from the least recent call of ReadLock.
    /// At this point that pointer is no longer valid, and should no longer be read.
    void ReadUnlock(void)  throw();

    /// Clear is not thread-safe and none of the lock or unlock functions should be called while it is running.
    void Clear(void) throw();

    /// This function will estimate how many elements are waiting to be read.  It's threadsafe enough that the value returned is stable, but not threadsafe enough to give accurate results.
    /// \return An ESTIMATE of how many data elements are waiting to be read
    int Size(void) const;

    /// Make sure that the pointer we done reading for the call to ReadUnlock is the right pointer.
    /// param[in] A previous pointer returned by ReadLock()
    bool CheckReadUnlockOrder(const ElementType* data) const;

    /// Returns if ReadUnlock was called before ReadLock
    /// \return If the read is locked
    bool ReadIsLocked(void) const;

private:
    struct DataPlusPtr
    {
        ElementType object;
        // Ready to read is so we can use an equality boolean comparison, in case the writePointer var is trashed while context switching.
        volatile bool readyToRead;
        volatile DataPlusPtr *next;
    };
    volatile DataPlusPtr *readAheadPointer;
    volatile DataPlusPtr *writeAheadPointer;
    volatile DataPlusPtr *readPointer;
    volatile DataPlusPtr *writePointer;
    volatile unsigned long unreadCount;
	volatile DataPlusPtr *readLastPointer;
	volatile unsigned long circleSize;
    volatile DataPlusPtr *idleTailPointer;
    volatile DataPlusPtr *idleHeadPointer;
	unsigned long minimumQueueSize;
	thd::CSpinLock rLock;
	thd::CSpinLock wLock;
};

template <class ElementType>
        CCircleQueue<ElementType>::CCircleQueue(unsigned long minimunSize/* = 8*/)  throw()
{
	wLock.Lock();
	rLock.Lock();
	minimumQueueSize = minimunSize;
    idleTailPointer = NULL;
    idleHeadPointer = idleTailPointer;
    // Preallocate
    readPointer = new DataPlusPtr;
    readPointer->readyToRead = false;
    readPointer->next = NULL;
    writePointer=readPointer;
    readPointer->next = new DataPlusPtr;
    readPointer->next->readyToRead = false;
    readPointer->next->next = NULL;
    int listSize;
#ifdef _DEBUG
    assert(minimumQueueSize >= 3);
#endif
    for (listSize=2; listSize < (int)minimumQueueSize; listSize++)
    {
        readPointer=readPointer->next;
        readPointer->next = new DataPlusPtr;
        readPointer->next->readyToRead = false;
        readPointer->next->next = NULL;
    }
	circleSize = minimumQueueSize;
    readPointer->next->next=writePointer; // last to next = start
    // set last read pointer
    readLastPointer = readPointer->next;
    readPointer=writePointer;
    readAheadPointer=readPointer;
    writeAheadPointer=writePointer;
    unreadCount=0;
	wLock.Unlock();
	rLock.Unlock();
}

template <class ElementType>
        CCircleQueue<ElementType>::~CCircleQueue()  throw()
{
	wLock.Lock();
	rLock.Lock();
	unreadCount = 0;
    volatile DataPlusPtr* next = NULL;
    readPointer = writeAheadPointer->next;
    while (readPointer!=writeAheadPointer)
    {
        next=readPointer->next;
        delete readPointer;
        readPointer = next;
    }
    delete readPointer;
	circleSize = 0;

    while(NULL != idleTailPointer)
    {
        next = idleTailPointer->next;
        delete idleTailPointer;
        idleTailPointer = next;
    }

    if(NULL != idleHeadPointer) {
        idleHeadPointer = NULL;
    }
	wLock.Unlock();
	rLock.Unlock();
}

template <class ElementType>
        ElementType* CCircleQueue<ElementType>::WriteLock( void ) throw()
{
	wLock.Lock();
    if (writeAheadPointer->next == readLastPointer ||
            writeAheadPointer->next->readyToRead==true)
    {
        volatile DataPlusPtr* newNextNode(NULL);
        if(idleHeadPointer == idleTailPointer
            || NULL == idleTailPointer){

            newNextNode = new DataPlusPtr;
            newNextNode->readyToRead = false;
            newNextNode->next = NULL;
        } else {
            newNextNode = idleTailPointer;
            atomic_xchgptr(&idleTailPointer, idleTailPointer->next);
        }
        assert(NULL != newNextNode);
        // set new node
        volatile DataPlusPtr *originalNext=writeAheadPointer->next;
        atomic_xchgptr(&writeAheadPointer->next, newNextNode);

        assert(writeAheadPointer->next);
        atomic_xchgptr(&writeAheadPointer->next->next, originalNext);

        atomic_inc(&circleSize);

    } else {

        if(idleHeadPointer != idleTailPointer) {

            DataPlusPtr *deletePointer = (DataPlusPtr *)idleTailPointer;
            atomic_xchgptr(&idleTailPointer, idleTailPointer->next);

            delete deletePointer;
            deletePointer = NULL;
        }
    }

    volatile DataPlusPtr *last(writeAheadPointer);
	atomic_xchgptr(&writeAheadPointer, writeAheadPointer->next);
    return (ElementType*) last;
}

template <class ElementType>
        void CCircleQueue<ElementType>::CancelWriteLock( ElementType* cancelToLocation )  throw()
{
    atomic_xchgptr(&writeAheadPointer, (DataPlusPtr *)cancelToLocation);
	wLock.Unlock();
}

template <class ElementType>
        void CCircleQueue<ElementType>::WriteUnlock( void )  throw()
{

#ifdef _DEBUG
    assert(writePointer->next!=readPointer);
    assert(writePointer!=writeAheadPointer);
#endif

	atomic_inc(&unreadCount);

    // User is done with the data, allow send by updating the write pointer
    atomic_xchg8(&writePointer->readyToRead, true);
    atomic_xchgptr(&writePointer, writePointer->next);

	wLock.Unlock();
}

template <class ElementType>
        ElementType* CCircleQueue<ElementType>::ReadLock( void )  throw()
{
	rLock.Lock();
    if (readAheadPointer==writePointer ||
            readAheadPointer->readyToRead==false)
    {
		rLock.Unlock();
        return 0;
    }

    volatile DataPlusPtr *last(readAheadPointer);
    atomic_xchgptr(&readAheadPointer, readAheadPointer->next);
    return (ElementType*)last;
}

template <class ElementType>
        void CCircleQueue<ElementType>::CancelReadLock( ElementType* cancelToLocation )  throw()
{
#ifdef _DEBUG
    assert(readPointer!=writePointer);
#endif
    atomic_xchgptr(&readAheadPointer, (DataPlusPtr *)cancelToLocation);
	rLock.Unlock();
}

template <class ElementType>
        void CCircleQueue<ElementType>::ReadUnlock( void )  throw()
{
#ifdef _DEBUG
    assert(readAheadPointer!=readPointer); // If hits, then called ReadUnlock before ReadLock
    assert(readPointer!=writePointer); // If hits, then called ReadUnlock when Read returns 0

    //if(readLastPointer->next != readPointer) {
    //    int nCount1 = 0;
    //    volatile DataPlusPtr *tempPointer1 = readLastPointer->next;
    //    while(tempPointer1 != readPointer) {
    //        ++nCount1;
    //        tempPointer1 = tempPointer1->next;
    //    }
    //    int nCount2 = 0;
    //    volatile DataPlusPtr *tempPointer2 = readPointer;
    //    while(tempPointer2 != readLastPointer->next) {
    //        ++nCount2;
    //        tempPointer2 = tempPointer2->next;
    //    }
    //}

    assert(readLastPointer->next == readPointer);
#endif

    long nLeaveSize = circleSize - atomic_dec(&unreadCount);
	if(
        (nLeaveSize > 2 || nLeaveSize < 1) // leave 2,1 empty node,don't push into delete list
		&& circleSize > (unsigned long)minimumQueueSize
/*            && readLastPointer->next == readPointer
        && readAheadPointer != readPointer
        && writePointer != readPointer*/)
    {
        atomic_xchgptr(&readLastPointer->next, readPointer->next);

        volatile DataPlusPtr *deletePointer = readPointer;
        atomic_xchgptr(&readPointer, readPointer->next);
        // reset data
        deletePointer->readyToRead = false;
        deletePointer->next = NULL;

        atomic_dec(&circleSize);

        // set delete list
        if(NULL == idleHeadPointer){
            atomic_xchgptr(&idleHeadPointer, deletePointer);
            atomic_xchgptr(&idleTailPointer, deletePointer);
        } else {
            atomic_xchgptr(&idleHeadPointer->next, deletePointer);
            atomic_xchgptr(&idleHeadPointer, idleHeadPointer->next);
        }
    }
	else
	{
		// set last read point
		atomic_xchgptr(&readLastPointer, readPointer);
        // Allow writes to this memory block
        atomic_xchg8(&readPointer->readyToRead, false);
		atomic_xchgptr(&readPointer, readPointer->next);
	}
	rLock.Unlock();
}

template <class ElementType>
        void CCircleQueue<ElementType>::Clear( void )  throw()
{
	wLock.Lock();
	rLock.Lock();
    // Shrink the list down to MINIMUM_LIST_SIZE elements
    writePointer = readPointer->next;
    volatile DataPlusPtr *next = NULL;

    int listSize=(int)circleSize;
    //next=readPointer->next;
    //while (next!=readPointer)
    //{
    //    listSize++;
    //    next=next->next;
    //}
    //assert(listSize == circleSize);

    while (listSize-- > (int)minimumQueueSize)
    {
        next=writePointer->next;
#ifdef _DEBUG
        assert(writePointer!=readPointer);
#endif
        delete writePointer;
        writePointer = next;
    }
    circleSize = minimumQueueSize;

    readPointer->next = writePointer;
    writePointer = readPointer;
    readAheadPointer = readPointer;
    writeAheadPointer = writePointer;
	unreadCount = 0;

    while(NULL != idleTailPointer)
    {
        next = idleTailPointer->next;
        delete idleTailPointer;
        idleTailPointer = next;
    }

    idleHeadPointer = NULL;
	wLock.Unlock();
	rLock.Unlock();
}

template <class ElementType>
        int CCircleQueue<ElementType>::Size(void) const
{
	return (int)unreadCount;
}

template <class ElementType>
        bool CCircleQueue<ElementType>::CheckReadUnlockOrder(const ElementType* data) const
{
    return const_cast<const ElementType *>(&readPointer->object) == data;
}

template <class ElementType>
        bool CCircleQueue<ElementType>::ReadIsLocked(void) const
{
    return readAheadPointer!=readPointer;
}

}

#endif /* CIRCLE_QUEUE */
