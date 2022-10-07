/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "DeadlockAvoidancePriorityQueue.h"
//#include "DeadlockAvoidanceSimulationManager.h"

//**************************************************************************************************************//
namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
        DeadlockAvoidancePriorityQueue::DeadlockAvoidancePriorityQueue(UInt32 capacity, OnDispose onDispose, OnBlockInformation *onBlock)
        {
            if(capacity == 0)
				throw ArgumentOutOfRangeException("capacity", "capacity is equal to zero.");

			this->onDispose = onDispose;
            this->capacity = capacity;

            mutex = new Mutex();
            queueNotEmpty = new ConditionVariable(mutex);
            queueNotFull = new ConditionVariable(mutex);
			rb = new RingBuffer<EventInterface *>(capacity);
			this->onBlock = onBlock; // Improve here with removing the need for a callback function call.

			peeked_event = null;
			blocked = false;
			lookahead = 0;
        }
        //----------------------------------------------------
        DeadlockAvoidancePriorityQueue::~DeadlockAvoidancePriorityQueue()
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
					while(rb->GetCount() != 0)
					{
						EventInterface *e = rb->Dequeue();
						onDispose(e);
					}
				}

				delete rb; rb = null;
                //disposed = true;
            }
        }
		//----------------------------------------------------
		/*void DeadlockAvoidancePriorityQueue::Enqueue(EventInterface *e)
		{
			mutex->Lock();
			{
				if(rb->GetCount() == 0)
				{
					rb->Enqueue(e);

					if(blocked)
					{
						//global_termination_detection_counter.Increment();///
						blocked = false;
						queueNotEmpty->Signal();
					}
				}
				else
					rb->Enqueue(e);
			}
			mutex->Unlock();
		}
		//----------------------------------------------------
		// Removes an item from the queue. Returns null if queue is empty.
		EventInterface *DeadlockAvoidancePriorityQueue::Dequeue()
		{

			EventInterface *e = peeked_event;
			peeked_event = null;
			return e;

			//mutex->Lock();
			//{
				//if(rb->GetCount() == 0)
				//{
				//	blocked = true;
				//	while(rb->GetCount() == 0)
				//		queueNotEmpty->Wait();
				//}

				//e = rb->Dequeue();

				//if(rb->GetCount() == capacity - 1)
				//	queueNotFull->Signal();
			//}
			//mutex->Unlock();

			//return e;
		}
		//----------------------------------------------------
		EventInterface *DeadlockAvoidancePriorityQueue::Peek()
		{
			if(peeked_event != null)
				return peeked_event;
			//EventInterface *e;

			mutex->Lock();
			{
				if(rb->GetCount() == 0)
				{
					blocked = true;
					onBlock->onBlock(onBlock->state, onBlock->OwnerClusterID);
					while(rb->GetCount() == 0)
						queueNotEmpty->Wait();
				}

				//e = rb->Peek();
				peeked_event = rb->Dequeue();

				//if(rb->GetCount() == capacity - 1)
				//	queueNotFull->Signal();
			}
			mutex->Unlock();

			return peeked_event;
		}//----------------------------------------------------
		UInt32 DeadlockAvoidancePriorityQueue::GetCount()
		{
			UInt32 count_ = 0;

			mutex->Lock();
			count_ = rb->GetCount();
			mutex->Unlock();

			return count_;
		}*/
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
