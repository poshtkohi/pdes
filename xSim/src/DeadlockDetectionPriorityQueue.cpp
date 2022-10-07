/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "DeadlockDetectionPriorityQueue.h"

//**************************************************************************************************************//
namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
        DeadlockDetectionPriorityQueue::DeadlockDetectionPriorityQueue(Int32 capacity, bool concurrentQueueWithBlocking, OnDispose onDispose)
        {
			if(capacity == 0)
				throw ArgumentOutOfRangeException("capacity", "capacity is equal to zero.");

			this->onDispose = onDispose;
            this->capacity = capacity;

            mutex = new Mutex();
            queueNotEmpty = new ConditionVariable(mutex);
            queueNotFull = new ConditionVariable(mutex);
			rb = new RingBuffer<EventInterface *>(capacity);

			blocked = false;
			peeked_event = null;
			ownerInputIndex = -1;
			count = LockFreeCounter(0);
			//disposed = false;
        }
        //----------------------------------------------------
        DeadlockDetectionPriorityQueue::~DeadlockDetectionPriorityQueue()
        {
            //if(!disposed)
            {
				queueNotEmpty->Broadcast();//
				queueNotFull->Broadcast();//
				mutex->Unlock();//
				delete mutex; mutex = null;
				delete queueNotEmpty; queueNotEmpty = null;
				delete queueNotFull; queueNotFull = null;

				if(onDispose != null)
				{
					EventInterface *e;

					while(rb->GetCount() != 0)
					{
						e = rb->Dequeue();
						onDispose(e);
					}
				}

				delete rb; rb = null;
                //disposed = true;
            }
        }
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
