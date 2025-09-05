
#ifndef SINGLE_PRODUCER_CONSUMER_H
#define SINGLE_PRODUCER_CONSUMER_H

#include <assert.h>
#include "SpinLock.h"
#include "SysTickCount.h"

namespace ntwk
{

    /// \brief A single producer consumer implementation without critical sections.
    template <class SingleProducerConsumerType, int KeepSize = 8, int KeepTimeMS = 300000>
    class SingleProducerConsumer
    {
    public:
        /// Constructor
        SingleProducerConsumer() throw();

        /// Destructor
        ~SingleProducerConsumer() throw();

        /// WriteLock must be immediately followed by WriteUnlock.  These two functions must be called in the same thread.
        /// \return A pointer to a block of data you can write to.
        SingleProducerConsumerType* WriteLock(void) throw();

        /// Call if you don't want to write to a block of data from WriteLock() after all.
        /// Cancelling locks cancels all locks back up to the data passed.  So if you lock twice and cancel using the first lock, the second lock is ignored
        /// \param[in] cancelToLocation Which WriteLock() to cancel.
        void CancelWriteLock(SingleProducerConsumerType* cancelToLocation) throw();

        /// Call when you are done writing to a block of memory returned by WriteLock()
        void WriteUnlock(void) throw();

        /// ReadLock must be immediately followed by ReadUnlock. These two functions must be called in the same thread.
        /// \retval 0 No data is availble to read
        /// \retval Non-zero The data previously written to, in another thread, by WriteLock followed by WriteUnlock.
        SingleProducerConsumerType* ReadLock(void) throw();

        // Cancelling locks cancels all locks back up to the data passed.  So if you lock twice and cancel using the first lock, the second lock is ignored
        /// param[in] Which ReadLock() to cancel.
        void CancelReadLock(SingleProducerConsumerType* cancelToLocation) throw();

        /// Signals that we are done reading the the data from the least recent call of ReadLock.
        /// At this point that pointer is no longer valid, and should no longer be read.
        void ReadUnlock(void) throw();

        /// Clear is not thread-safe and none of the lock or unlock functions should be called while it is running.
        void Clear(void) throw();

        /// This function will estimate how many elements are waiting to be read.  It's threadsafe enough that the value returned is stable, but not threadsafe enough to give accurate results.
        /// \return An ESTIMATE of how many data elements are waiting to be read
        int Size(void) const;

        /// Make sure that the pointer we done reading for the call to ReadUnlock is the right pointer.
        /// param[in] A previous pointer returned by ReadLock()
        bool CheckReadUnlockOrder(const SingleProducerConsumerType* data) const;

        /// Returns if ReadUnlock was called before ReadLock
        /// \return If the read is locked
        bool ReadIsLocked(void) const;

    private:
        struct DataPlusPtr
        {
            SingleProducerConsumerType object;

            volatile DataPlusPtr *next;

			uint32_t activeTime;
            // Ready to read is so we can use an equality boolean comparison, in case the writePointer var is trashed while context switching.
            volatile bool readyToRead;
        };
        volatile DataPlusPtr *readAheadPointer;
        volatile DataPlusPtr *writeAheadPointer;
        volatile DataPlusPtr *readPointer;
        volatile DataPlusPtr *writePointer;
		volatile DataPlusPtr *readLastPointer;
        volatile DataPlusPtr *idleTailPointer;
        volatile DataPlusPtr *idleHeadPointer;
		volatile unsigned long unreadCount;
		volatile unsigned long circleSize;
		thd::CSpinLock rLock;
		thd::CSpinLock wLock;
    };

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::SingleProducerConsumer() throw()
    {
		wLock.Lock();
		rLock.Lock();
        idleTailPointer = NULL;
        idleHeadPointer = idleTailPointer;
        // Preallocate
        readPointer = new DataPlusPtr;
        readPointer->next = NULL;
        readPointer->activeTime = static_cast<uint32_t>(GetSysTickCount());
        readPointer->readyToRead = false;
        writePointer=readPointer;
        readPointer->next = new DataPlusPtr;
		readPointer->next->next = NULL;
        readPointer->next->activeTime = static_cast<uint32_t>(GetSysTickCount());
        readPointer->next->readyToRead = false;
        
        int listSize;
#ifdef _DEBUG
        assert(KeepSize >= 3);
#endif
        for(listSize=2; listSize < KeepSize; listSize++)
        {
            readPointer=readPointer->next;
            readPointer->next = new DataPlusPtr;
			readPointer->next->next = NULL;
            readPointer->next->activeTime = static_cast<uint32_t>(GetSysTickCount());
            readPointer->next->readyToRead = false;

        }
		circleSize = KeepSize;
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

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::~SingleProducerConsumer() throw()
    {
		wLock.Lock();
		rLock.Lock();
		unreadCount = 0;
        volatile DataPlusPtr *next = NULL;
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

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            SingleProducerConsumerType* SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::WriteLock( void ) throw()
    {
		wLock.Lock();
        if (writeAheadPointer->next == readLastPointer ||
                writeAheadPointer->next->readyToRead==true)
        {
            volatile DataPlusPtr *newNextNode(NULL);
            if(idleHeadPointer == idleTailPointer
                || NULL == idleTailPointer){

                newNextNode = new DataPlusPtr;
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

        return (SingleProducerConsumerType*) last;
    }

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            void SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::CancelWriteLock( SingleProducerConsumerType* cancelToLocation ) throw()
    {
        atomic_xchgptr(&writeAheadPointer, (DataPlusPtr *)cancelToLocation);
		wLock.Unlock();
    }

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            void SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::WriteUnlock( void ) throw()
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

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            SingleProducerConsumerType* SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::ReadLock( void ) throw()
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
		if(!last) {
			rLock.Unlock();
		}
        return (SingleProducerConsumerType*)last;
    }

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            void SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::CancelReadLock( SingleProducerConsumerType* cancelToLocation ) throw()
    {
#ifdef _DEBUG
        assert(readPointer!=writePointer);
#endif
        atomic_xchgptr(&readAheadPointer, (DataPlusPtr *)cancelToLocation);
		rLock.Unlock();
    }

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            void SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::ReadUnlock( void ) throw()
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
			&& circleSize > (unsigned long)KeepSize
            && (static_cast<uint32_t>(GetSysTickCount()) - readPointer->activeTime) > KeepTimeMS)
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

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            void SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::Clear( void ) throw()
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

        while (listSize-- > KeepSize)
        {
            next=writePointer->next;
#ifdef _DEBUG
            assert(writePointer!=readPointer);
#endif
            delete writePointer;
            writePointer = next;
        }
        circleSize = KeepSize;

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

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            int SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::Size(void) const
    {
        return (int)unreadCount;
    }

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            bool SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::CheckReadUnlockOrder(const SingleProducerConsumerType* data) const
    {
        return const_cast<const SingleProducerConsumerType *>(&readPointer->object) == data;
    }

    template <class SingleProducerConsumerType, int KeepSize, int KeepTimeMS>
            bool SingleProducerConsumer<SingleProducerConsumerType, KeepSize, KeepTimeMS>::ReadIsLocked(void) const
    {
        return readAheadPointer!=readPointer;
    }
}

#endif /* SINGLE_PRODUCER_CONSUMER_H */
