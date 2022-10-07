/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_ProcessingElement_h__
#define __Parvicursor_xSim_ProcessingElement_h__

#include <general.h>
#include <StaticFunctions/StaticFunctions.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/DateTime/DateTime.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>
#include <System/InvalidOperationException/InvalidOperationException.h>

#include "BinaryHeap.h"
#include "BlockingRingBuffer.h"
#include "LogicalProcess.h"
#include "EventInterface.h"
#include "InternalEvent.h"
#include "Vector.h"
#include "SortedList.h"
#include "State.h"

using namespace System;
using namespace Parvicursor::xSim::Collections;
//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        // Forward declarations.
		//class LogicalProcess;

        class ProcessingElement : public Object
        {
            friend class TimeWarpSimulationManager;
			/*---------------------fields-----------------*/
			// The ID of the processing element.
			private: Int32 peID;
			//private: RingBuffer<EventInterface *> *localInputChannel;
			private: BlockingRingBuffer<EventInterface *> *inputChannel;
			private: EventInterface *dequeuedMessage;
			private: BinaryHeap<EventInterface *> *unprocessedQueue;
			//private: SortedList<EventIterface *> *outputQueue;
			// The period to prepare checkpoints based on periodic copy state saving (Infrequent State Saving).
		    // Default is 1.
			//private: Int32 stateSavingPeriod;
			// periodicStateSavingCounter must be set to zero at the beginning of the simulation, before 
			// each rollback or reaching the value of stateSavingPeriod.
			// It indicates when we must prepare a checkpoint.
			private: Int32 periodicStateSavingCounter;
			private: UInt64 numberOfProcessedEvents;
			private: UInt64 localVirtualTime;
			private: enum InputChannelState { NormalState = 0, DeadlockState = 1};
			private: InputChannelState inputChannelState;
            private: bool disposed;
			/*---------------------methods----------------*/
            // ProcessingElement Class constructor.
            public: ProcessingElement(Int32 PeID);
            // ProcessingElement Class destructor.
            public: ~ProcessingElement();
			// Enqueues an event into inputChannel of the processing element.
			public: /*inline*/ void EnqueueMessage(EventInterface *e);
			// Dequeues an event from inputChannel of the processing element.
			public: /*inline*/ EventInterface *DequeueMessage();
			public: inline Int32 GetPeID() { return peID; }
			///public: inline SortedList *GetStateQueue() { return Processed; }
			///public: inline SortedList *GetOutputQueue() { return outputQueue; }
			public: inline UInt64 GetNumberOfProcessedEvents() { return numberOfProcessedEvents; }
			public: inline void SetNumberOfProcessedEvents(UInt64 newVal) { numberOfProcessedEvents = newVal; }
			// A callback to be called by unprocessedQueue when removing the item which contains a positive message matched by the received antimessage.
			public: static void OnRemoveUnprocessedQueue(EventInterface *e);
			public: static void OnDisposeProcessedOrOutputQueue(EventInterface *e);
			public: static void OnRemoveNegQueue(EventInterface *e);
			// This method is called when dequeuing a message from inputChannel.
			// It implements the main Time Warp functionality.
			private: static void OnDequeueInputChannel(EventInterface *e);
		    // This method is called when rolling back the processed queue of an LP.
			private: static void OnRemoveProcessedQueue(EventInterface *e);
			// This method is called when rolling back the processed queue of an LP.
			private: static void OnRemoveOutputQueue(EventInterface *e);
			// This method is called when rolling back the processed queue of an LP upon receiving an antimessage in the past.
			private: static void OnRemoveProcessedQueueAntiMessageReceive(EventInterface *e);
			// A callback to be called by SortedList class when removing the items in the outputQueue.
			///public: static void OnRemoveOutputQueue(const EventInterface *data);
			// Sets the period of state saving for periodic copy state saving.
			//private: inline void SetStateSavingPeriod(Int32 newVal) { stateSavingPeriod = newVal; }
			// Gets the period of state saving for periodic copy state saving.
			//private: inline Int32 GetStateSavingPeriod() { return stateSavingPeriod; }
			// Sets the periodicStateSavingCounter for periodic copy state saving.
			// It indicates when we must prepare a checkpoint.
			//private: inline void SetPeriodicStateSavingCounter(Int32 newVal) { periodicStateSavingCounter = newVal; }
			// Gets the periodicStateSavingCounter for periodic copy state saving.
		    // It indicates when we must prepare a checkpoint.
			//private: inline Int32 GetStateSavingCounter() { return periodicStateSavingCounter; }
			// Sets local virtual time of the processing element.
			//private: inline void SetLocalVirtualTime(UInt64 newVal) { localVirtualTime = newVal; }
			// Gets local virtual time of the processing element.
			//private: inline UInt64 GetLocalVirtualTime() { return localVirtualTime; }
        };

	}
};
//**************************************************************************************************************//

#endif

