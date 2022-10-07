/**
	#define meta ...
	printf("%s\n", meta);
**/

/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_DeadlockAvoidancePriorityQueue_h__
#define __Parvicursor_xSim_DeadlockAvoidancePriorityQueue_h__

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

//#include "DeadlockAvoidancePriorityQueueInterface.h"

#include <iostream>
#include <sstream>
#include <fstream>

using std::string;
using namespace std;

using namespace System;
using namespace System::Threading;

using namespace LockFree;

#include "EventInterface.h"
#include "SimulationManagerInterface.h"
#include "RingBuffer.h"

using namespace Parvicursor::xSim::Collections;

extern LockFreeCounter global_termination_detection_counter;
extern Mutex *mtx;


//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------

		// the Callback definition.
		typedef void (*OnBlock)(Object *state, UInt32 OwnerClusterID);
		struct OnBlockInformation
		{
			Object *state;
			UInt32 OwnerClusterID;
			OnBlock onBlock;
		};
		typedef void (*OnDispose)(EventInterface *data);

		// Forward declarations
		class DeadlockAvoidanceSimulationManager;

		class DeadlockAvoidancePriorityQueue: public Object// : public DeadlockAvoidancePriorityQueueInterface // For performance improvements, we may not use the QueueInterface.
		{
			friend class DeadlockAvoidanceSimulationManager;
			/*---------------------fields----------------*/
			private: RingBuffer<EventInterface *> *rb;
			private: UInt32 count;
			private: UInt32 capacity;
			private: Mutex *mutex;
			private: ConditionVariable *queueNotEmpty, *queueNotFull;
			private: EventInterface *peeked_event;
			private: UInt32 ownerClusterID;
			private: SimulationManagerInterface *manager;
			private: OnDispose onDispose;
			private: OnBlockInformation *onBlock;
			private: UInt32 lookahead;
			private: bool blocked;
			/*---------------------methods----------------*/
			// DeadlockAvoidancePriorityQueue Class constructor.
			public: DeadlockAvoidancePriorityQueue(UInt32 capacity, OnDispose onDispose, OnBlockInformation *onBlock);
			// DeadlockAvoidancePriorityQueue Class destructor.
			public: ~DeadlockAvoidancePriorityQueue();
			public: inline void SetOwnerClusterID(UInt32 owner) { ownerClusterID = owner; }
			public: inline UInt32 GetOwnerClusterID() { return ownerClusterID; }
			public: inline void SetSimulationManager(SimulationManagerInterface *managerInstance) { manager = managerInstance; }
			// Removes all items from the queue
			public: void ClearDeadlockAvoidancePriorityQueue();
			// Enters an item in the queue
			public: inline void Enqueue(EventInterface *e)
			{
				mutex->Lock();
				{
					if(rb->GetCount() == 0)
					{
						rb->Enqueue(e);
						queueNotEmpty->Signal();
					}
					else
						rb->Enqueue(e);
				}
				mutex->Unlock();
			}
			// Removes an item from the queue. Returns null if queue is empty.
			public: inline EventInterface *Dequeue()
			{
				EventInterface *e = peeked_event;
				peeked_event = null;
				return e;
			}
			public: inline EventInterface *Peek()
			{
				if(peeked_event != null)
					return peeked_event;

				mutex->Lock();
				{
					if(rb->GetCount() == 0)
					{
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
			}
			public: UInt32 GetCount()
			{

				UInt32 count_ = 0;

				mutex->Lock();
				count_ = rb->GetCount();
				mutex->Unlock();

				return count_;
			}
			//----------------------------------------------------
		};
	}
};
//**************************************************************************************************************//

#endif

