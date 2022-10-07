/**
    #define meta ...
    printf("%s\n", meta);
**/

/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_BlockingRingBuffer_h__
#define __Parvicursor_xSim_BlockingRingBuffer_h__

#include <general.h>
#include <StaticFunctions/StaticFunctions.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/Convert/Convert.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System.Threading/Mutex/Mutex.h>
#include <System.Threading/ConditionVariable/ConditionVariable.h>

#include <../Samples/LockFree/LockFree/LockFreeCounter/LockFreeCounter.h>

#include "EventInterface.h"
#include "LogicalProcess.h"
#include "RingBuffer.h"

using namespace System;
using namespace System::Threading;
using namespace Parvicursor::xSim::Collections;
using namespace LockFree;


extern LockFreeCounter tw_global_termination_counter;
extern Mutex *tw_global_mutex_controller;
extern ConditionVariable *tw_global_waitcv_controller;
extern bool tw_global_met;
//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		namespace Collections
		{
			//----------------------------------------------------
			template <class T>
			class BlockingRingBuffer: public Object
			{
				public: typedef void (*OnDequeue)(T val);
				/*---------------------fields----------------*/
				private: bool blocked;
				private: bool disposed;
				private: RingBuffer<T> *rb;
				private: Mutex *mutex;
				private: ConditionVariable *queueNotEmpty, *queueNotFull;
				/*---------------------methods----------------*/
				// BlockingRingBuffer Class constructor.
				public: BlockingRingBuffer(UInt32 capacity)
				{
					disposed = true;
					rb = new RingBuffer<T>(capacity);

					//count = LockFreeCounter();
					mutex = new Mutex();
					queueNotEmpty = new ConditionVariable(mutex);
					queueNotFull = new ConditionVariable(mutex);
					blocked = false;
					disposed = false;
				}
				//----------------------------------------------------
				// BlockingRingBuffer Class destructor.
				public: ~BlockingRingBuffer()
				{
					if(!disposed)
					{
						disposed = true;
						queueNotEmpty->Broadcast();//
						queueNotFull->Broadcast();//
						mutex->Unlock();//
						delete queueNotEmpty;
						delete queueNotFull;
						delete mutex;
						delete rb;
						// Consider here deleting unprocessed messafes.
						//printf("BlockingRingBuffer() Allocations: %d\n", allocations);
					}
				}
				//----------------------------------------------------
				// Removes all items from the queue
				public: inline void Clear()
				{
					rb->Clear();
				}
				//----------------------------------------------------
				// Enters an item in the queue
				public: inline void Enqueue(const T &val)
				{
					// The buffer is full.
					//if(count == capacity)
					//	return false;

					//count.Increment();
					//return true;

					mutex->Lock();
					{
						if(rb->GetCount() == 0 && blocked)
						{
							///cout << "Enqueue 1" << endl;
							tw_global_termination_counter.Increment();
							blocked = false;
						}

						/*while(count == capacity)
						{
							//blocked = true;//
							EventInterface *e = (EventInterface *)obj;
							cout << "Enqueue 2 srcPe " << e->GetSenderReference()->GetOwnerClusterID() << endl;
							queueNotFull->Wait();
						}*/

						rb->Enqueue(val);

						if(rb->GetCount() - 1 == 0)
							queueNotEmpty->Broadcast();

					}
					mutex->Unlock();
				}
				//----------------------------------------------------
				// Removes an item from the queue. Returns null if queue is empty.
				public: inline const T &Dequeue(Int32 &canBlock)
				{
					// The buffer is empty.
					//if(count == 0)
					//	return null;

					T val;

					mutex->Lock();
					{
						if(rb->GetCount() == 0)
						{
							if(!canBlock)
							{
								canBlock = -1;
								mutex->Unlock();
								return val;
							}
							blocked = true;
							Long count_ = tw_global_termination_counter.Decrement_And_get_Count();
							if(count_ == 0)
							{
								///cout << "Dequeue 1" << endl;
								tw_global_mutex_controller->Lock();
								tw_global_met = true;
								tw_global_waitcv_controller->Signal();
								tw_global_mutex_controller->Unlock();
							}
						}
						while(rb->GetCount() == 0)
							queueNotEmpty->Wait();


						/*if(rb->GetCount() == capacity)
						{
							///cout << "Dequeue 2" << endl;
							queueNotFull->Broadcast();
						}*/

						val = rb->Dequeue();
					}
					mutex->Unlock();

					return val;
				}
				//----------------------------------------------------
				// Removes an item from the queue. Returns null if queue is empty.
				public: void DequeueAll(OnDequeue onDequeue, Int32 &canBlock)
				{
					// The buffer is empty.
					//if(count == 0)
					//	return null;

					T val;

					mutex->Lock();
					{
						if(rb->GetCount() == 0)
						{
							if(!canBlock)
							{
								canBlock = -1;
								mutex->Unlock();
								return;
							}
							blocked = true;
							Long count_ = tw_global_termination_counter.Decrement_And_get_Count();
							if(count_ == 0)
							{
								///cout << "Dequeue 1" << endl;
								tw_global_mutex_controller->Lock();
								tw_global_met = true;
								tw_global_waitcv_controller->Signal();
								tw_global_mutex_controller->Unlock();
							}
						}
						while(rb->GetCount() == 0)
							queueNotEmpty->Wait();


						/*if(rb->GetCount() == capacity)
						{
						///cout << "Dequeue 2" << endl;
						queueNotFull->Broadcast();
						}*/

						while(rb->GetCount() != 0)
						{
							val = rb->Dequeue();
							onDequeue(val);
						} 
					}
					mutex->Unlock();
				}
				//----------------------------------------------------
				public: inline const T &Peek()
				{
					// The buffer is empty.
					//if(count.get_Count() == 0)
					//	return null;

					T val;

					mutex->Lock();
					{
						while(rb->GetCount() == 0)
							queueNotEmpty->Wait();

						val = rb->Peek();
					}
					mutex->Unlock();

					return val;
				}
				//----------------------------------------------------
				public: inline Int32 GetCount()
				{
					//return count.get_Count();
					Int32 _count;
					mutex->Lock();
					_count = rb->GetCount();
					mutex->Unlock();
					return _count;
				}
				//private: inline Int32 GetCountInternal() { return count + count_deltas; }
				//----------------------------------------------------
			};
		}
	}
};
//**************************************************************************************************************//

#endif

