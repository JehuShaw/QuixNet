
#ifndef CIRCLEQUEUE_H
#define CIRCLEQUEUE_H

#include <assert.h>
#include "SpinLock.h"
#include "SysTickCount.h"

namespace thd {

/// \brief A single producer consumer implementation without critical sections.
template <typename DataType, int NodeKeepSize = 8, int NodeKeepTimeMS = 300000>
class CCircleQueue
{
public:
    /// Constructor
    CCircleQueue()  throw();

    /// Destructor
    ~CCircleQueue()  throw();

    /// WriteLock must be immediately followed by WriteUnlock.  These two functions must be called in the same thread.
    /// \return A pointer to a block of data you can write to.
    DataType* WriteLock(void)  throw();

    /// Call if you don't want to write to a block of data from WriteLock() after all.
    /// Cancelling locks cancels all locks back up to the data passed.  So if you lock twice and cancel using the first lock, the second lock is ignored
    /// \param[in] cancelToLocation Which WriteLock() to cancel.
    void CancelWriteLock(DataType* cancelToLocation)  throw();

    /// Call when you are done writing to a block of memory returned by WriteLock()
    void WriteUnlock(void)  throw();

    /// ReadLock must be immediately followed by ReadUnlock. These two functions must be called in the same thread.
    /// \retval 0 No data is availble to read
    /// \retval Non-zero The data previously written to, in another thread, by WriteLock followed by WriteUnlock.
    DataType* ReadLock(void)  throw();

    // Cancelling locks cancels all locks back up to the data passed.  So if you lock twice and cancel using the first lock, the second lock is ignored
    /// param[in] Which ReadLock() to cancel.
    void CancelReadLock(DataType* cancelToLocation)  throw();

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
    bool CheckReadUnlockOrder(const DataType* data) const;

    /// Returns if ReadUnlock was called before ReadLock
    /// \return If the read is locked
    bool ReadIsLocked(void) const;

private:
    struct CircleNode
    {
        DataType object;

        volatile CircleNode* next;

        uint32_t activeTime;
        // Ready to read is so we can use an equality boolean comparison, in case the writePointer var is trashed while context switching.
        volatile bool readyToRead;
    };
    volatile CircleNode* readAheadPointer;
    volatile CircleNode* writeAheadPointer;
    volatile CircleNode* readPointer;
    volatile CircleNode* writePointer;
	volatile CircleNode* readLastPointer;
    volatile CircleNode* idleTailPointer;
    volatile CircleNode* idleHeadPointer;
    volatile unsigned long unreadCount;
    volatile unsigned long circleSize;
	thd::CSpinLock rLock;
	thd::CSpinLock wLock;
};

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::CCircleQueue()  throw()
{
	wLock.Lock();
	rLock.Lock();
    idleTailPointer = NULL;
    idleHeadPointer = idleTailPointer;
    // Preallocate
    readPointer = new CircleNode;
    readPointer->next = NULL;
    readPointer->activeTime = static_cast<uint32_t>(GetSysTickCount());
    readPointer->readyToRead = false;
    writePointer=readPointer;
    readPointer->next = new CircleNode;
    readPointer->next->next = NULL;
    readPointer->next->activeTime = static_cast<uint32_t>(GetSysTickCount());
    readPointer->next->readyToRead = false;
    int listSize;
#ifdef _DEBUG
    assert(NodeKeepSize >= 3);
#endif
    for (listSize=2; listSize < NodeKeepSize; listSize++)
    {
        readPointer=readPointer->next;
        readPointer->next = new CircleNode;
        readPointer->next->next = NULL;
        readPointer->next->activeTime = static_cast<uint32_t>(GetSysTickCount());
        readPointer->next->readyToRead = false;
    }
	circleSize = NodeKeepSize;
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

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::~CCircleQueue()  throw()
{
	wLock.Lock();
	rLock.Lock();
	unreadCount = 0;
    volatile CircleNode* next = NULL;
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

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        DataType* CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::WriteLock( void ) throw()
{
	wLock.Lock();
    if (writeAheadPointer->next == readLastPointer ||
            writeAheadPointer->next->readyToRead==true)
    {
        volatile CircleNode* newNextNode(NULL);
        if(idleHeadPointer == idleTailPointer
            || NULL == idleTailPointer){

            newNextNode = new CircleNode;
            newNextNode->next = NULL;
            newNextNode->activeTime = static_cast<uint32_t>(GetSysTickCount());
            newNextNode->readyToRead = false;
        } else {
            newNextNode = idleTailPointer;
            newNextNode->activeTime = static_cast<uint32_t>(GetSysTickCount());
            atomic_xchgptr(&idleTailPointer, idleTailPointer->next);
        }
        assert(NULL != newNextNode);
        // set new node
        volatile CircleNode* originalNext=writeAheadPointer->next;
        atomic_xchgptr(&writeAheadPointer->next, newNextNode);

        assert(writeAheadPointer->next);
        atomic_xchgptr(&writeAheadPointer->next->next, originalNext);

        atomic_inc(&circleSize);

    } else {

        if (idleHeadPointer != idleTailPointer) {

            volatile CircleNode* deletePointer = idleTailPointer;
            atomic_xchgptr(&idleTailPointer, idleTailPointer->next);

            delete deletePointer;
            deletePointer = NULL;
        }
    }

    volatile CircleNode* last(writeAheadPointer);
	atomic_xchgptr(&writeAheadPointer, writeAheadPointer->next);
    return (DataType*) last;
}

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        void CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::CancelWriteLock( DataType* cancelToLocation )  throw()
{
    atomic_xchgptr(&writeAheadPointer, (CircleNode *)cancelToLocation);
	wLock.Unlock();
}

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        void CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::WriteUnlock( void )  throw()
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

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        DataType* CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::ReadLock( void )  throw()
{
	rLock.Lock();
    if (readAheadPointer==writePointer ||
            readAheadPointer->readyToRead==false)
    {
		rLock.Unlock();
        return 0;
    }

    volatile CircleNode* last(readAheadPointer);
    atomic_xchgptr(&readAheadPointer, readAheadPointer->next);
	if (!last) {
		rLock.Unlock();
	}
    return (DataType*)last;
}

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        void CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::CancelReadLock( DataType* cancelToLocation )  throw()
{
#ifdef _DEBUG
    assert(readPointer!=writePointer);
#endif
    atomic_xchgptr(&readAheadPointer, (CircleNode* )cancelToLocation);
	rLock.Unlock();
}

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        void CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::ReadUnlock( void )  throw()
{
#ifdef _DEBUG
    assert(readAheadPointer!=readPointer); // If hits, then called ReadUnlock before ReadLock
    assert(readPointer!=writePointer); // If hits, then called ReadUnlock when Read returns 0

    //if(readLastPointer->next != readPointer) {
    //    int nCount1 = 0;
    //    volatile CircleNode *tempPointer1 = readLastPointer->next;
    //    while(tempPointer1 != readPointer) {
    //        ++nCount1;
    //        tempPointer1 = tempPointer1->next;
    //    }
    //    int nCount2 = 0;
    //    volatile CircleNode *tempPointer2 = readPointer;
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
		&& circleSize > (unsigned long)NodeKeepSize
        && (static_cast<uint32_t>(GetSysTickCount()) - readPointer->activeTime) > NodeKeepTimeMS)
    {
        atomic_xchgptr(&readLastPointer->next, readPointer->next);

        volatile CircleNode* deletePointer = readPointer;
        atomic_xchgptr(&readPointer, readPointer->next);
        // reset data
        deletePointer->next = NULL;
        deletePointer->readyToRead = false;

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

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        void CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::Clear( void )  throw()
{
	wLock.Lock();
	rLock.Lock();
    // Shrink the list down to MINIMUM_LIST_SIZE elements
    writePointer = readPointer->next;
    volatile CircleNode* next = NULL;

    int listSize = (int)circleSize;
    //next=readPointer->next;
    //while (next!=readPointer)
    //{
    //    listSize++;
    //    next=next->next;
    //}
    //assert(listSize == circleSize);

    while (listSize-- > NodeKeepSize)
    {
        next=writePointer->next;
#ifdef _DEBUG
        assert(writePointer!=readPointer);
#endif
        delete writePointer;
        writePointer = next;
    }
    circleSize = NodeKeepSize;

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

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        int CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::Size(void) const
{
	return (int)unreadCount;
}

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        bool CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::CheckReadUnlockOrder(const DataType* data) const
{
    return const_cast<const DataType *>(&readPointer->object) == data;
}

template <typename DataType, int NodeKeepSize, int NodeKeepTimeMS>
        bool CCircleQueue<DataType, NodeKeepSize, NodeKeepTimeMS>::ReadIsLocked(void) const
{
    return readAheadPointer!=readPointer;
}

}

#endif /* CIRCLEQUEUE_H */
