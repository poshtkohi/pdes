/**
	#define meta ...
	printf("%s\n", meta);
**/

/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_DeadlockDetectionPriorityQueue_h__
#define __Parvicursor_xSim_DeadlockDetectionPriorityQueue_h__

#include <general.h>
#include <StaticFunctions/StaticFunctions.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/Convert/Convert.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System.Threading/Thread/Thread.h>
#include <System.Threading/Mutex/Mutex.h>
#include <System.Threading/ConditionVariable/ConditionVariable.h>
#include <../Samples/LockFree/LockFree/LockFreeCounter/LockFreeCounter.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//#include "PriorityQueueInterface.h"

#include <iostream>
#include <sstream>
#include <fstream>

using std::string;
using namespace std;

using namespace System;
using namespace System::Threading;

using namespace LockFree;

#define DeltaQueueCapacity 8192

/*inline static void prefetch0(void *addr)
{
__asm {
movlps eax, addr		// Load the address of the addr variable into edx.
PREFETCHT0 [eax]
}
}*/

#include "EventInterface.h"
#include "SimulationManagerInterface.h"
#include "RingBuffer.h"

using namespace Parvicursor::xSim::Collections;

extern LockFreeCounter global_deadlock_counter;
extern Mutex *global_mutex_controller;
extern ConditionVariable *global_waitcv_controller;
extern bool global_met;
//extern volatile int global_deadlock_counter_;

//extern pthread_spinlock_t global_counter_lock;

extern Mutex *mtx;
#define MaxRingBufferSize 1024*1024


//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------

		// the Callback definition.
		typedef void (*QueueStateNotificationCallback)(Object *, UInt32);
		typedef void (*OnDispose)(EventInterface *data);

		// Forward declarations
		class DeadlockDetectionSimulationManager;

		class DeadlockDetectionPriorityQueue: public Object// : public PriorityQueueInterface // For performance improvements, we may not use the QueueInterface.
		{
			friend class DeadlockDetectionSimulationManager;
			/*---------------------fields----------------*/
			private: RingBuffer<EventInterface *> *rb;
			private: UInt32 capacity;
			private: Mutex *mutex;
			private: ConditionVariable *queueNotEmpty, *queueNotFull;
			private: bool concurrentQueueWithBlocking;
			private: bool blocked;
			private: EventInterface *peeked_event;
			private: UInt64 peeked_timestamp;
			private: bool first;
			private: bool fromDeadlockRecovery;
			private: UInt32 ownerClusterID;
			private: UInt32 ownerInputIndex;
			private: SimulationManagerInterface *manager;
			private: OnDispose onDispose;
			private: LockFreeCounter count;
			/*---------------------methods----------------*/
			// DeadlockDetectionPriorityQueue Class constructor.
			public: DeadlockDetectionPriorityQueue(Int32 capacity, bool concurrentQueueWithBlocking, OnDispose onDispose);
			// DeadlockDetectionPriorityQueue Class destructor.
			public: ~DeadlockDetectionPriorityQueue();
			public: inline void SetOwnerClusterID(Int32 owner) { ownerClusterID = owner; }
			public: inline Int32 GetOwnerClusterID() { return ownerClusterID; }
			public: inline void SetSimulationManager(SimulationManagerInterface *managerInstance) { manager = managerInstance; }
			// Removes all items from the queue
			public: void ClearPriorityQueue();
			// Enters an item in the queue
			public: inline void Enqueue(EventInterface *e)
			{
				mutex->Lock();
				{
					if(rb->GetCount() == 0)
					{
						rb->Enqueue(e);

						if(blocked)
						{
							//std::cout << "DeadlockDetectionPriorityQueue::Enqueue() blocked=true" << endl;
							global_deadlock_counter.Increment();///
							manager->SetBlockedPriorityQueue(GetOwnerClusterID(), null);
							blocked = false;
							queueNotEmpty->Signal();
						}
					}
					else
						rb->Enqueue(e);

				}
				mutex->Unlock();

				count.Increment();
			}
			// Removes an item from the queue. Returns null if queue is empty.
			public: inline EventInterface *Dequeue()
			{
				EventInterface *e = peeked_event;
				peeked_event = null;
				count.Decrement();
				return e;
			}
			// Deadlock in the testbed is easily detected by maintaining a global counter indicating the
			// number of processes that are either scheduled or running. The counter is incremented whenever a
			// process unblocks another, and decremented whenever a process becomes blocked. Once a process
			// decrements the counter, it will not increment it again until it has been rescheduled and
			// resumes execution. The system is deadlocked whenever the counter reaches zero and there is at
			// least one process that has not yet terminated (otherwise, the computation has terminated). Each
			// scheduler checks if the deadlock counter has reached zero whenever it fails to find a process to
			// run. If the counter is zero, the scheduler initiates a computation to break the deadlock.
			public: inline EventInterface *Peek()
			{
				if(peeked_event != null)
					return peeked_event;

				mutex->Lock();
				{
					if(rb->GetCount() == 0)
					{
						//printf("****** a deadlock was detected ********\n");
						blocked = true;
						manager->SetBlockedPriorityQueue(GetOwnerClusterID(), this);
						Long count_ = global_deadlock_counter.Decrement_And_get_Count();
						//peeked_event = null;
						if(count_ == 0)
						{
							//mtx->Lock(); printf("=============================================\n"); mtx->Unlock();
							//mtx->Lock(); printf("A deadlock was detected on DeadlockDetectionPriorityQueue::Peek() in Cluster %d\n", GetOwnerClusterID()); mtx->Unlock();
							//exit(0);
							global_mutex_controller->Lock();
							global_met = true;
							global_waitcv_controller->Signal();
							global_mutex_controller->Unlock();
						}

						//while(count + count_deltas == 0)
						while(rb->GetCount() == 0)
						//while(count == 0)
							queueNotEmpty->Wait();
					}

					//obj = PeekInternal(timestamp);
					peeked_event = rb->Dequeue();

					/*if(count == capacity - 1)
						queueNotFull->Signal();*/
				}
				mutex->Unlock();

				return peeked_event;
			}

			public: inline UInt32 GetCount()
			{
                return count.get_Count();
				/*UInt32 count_ = 0;

				mutex->Lock();
				count_ = rb->GetCount();
				mutex->Unlock();

				return count_;*/
			}
			//private: inline Int32 GetArrayIndex(Int32 bufferIndex);
			//----------------------------------------------------
		};
	}
};
//**************************************************************************************************************//

#endif

