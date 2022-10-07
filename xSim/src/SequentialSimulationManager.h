/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_SequentialSimulationManager_h__
#define __Parvicursor_xSim_SequentialSimulationManager_h__

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
#include <System.Threading/Thread/Thread.h>
#include <System.Threading/ThreadStateException/ThreadStateException.h>
#include <Parvicursor/Profiler/ResourceProfiler.h>

#include "LogicalProcess.h"
#include "SimulationManagerType.h"
#include "SimulationManagerInterface.h"
#include "priority_queue.h"
#include "EventInterface.h"
#include "Vector.h"
#include "SpaceTimeGraph.h"
#include "Globals.h"

using namespace System;
using namespace System::Threading;
using namespace Parvicursor::Profiler;
using namespace Parvicursor::xSim::psc;
using namespace Parvicursor::PS2;

#include <iostream>
#include <vector>
//#include <unordered_map>

using namespace std;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		class SequentialSimulationManager : public SimulationManagerInterface
		{
            /*---------------------fields-----------------*/
			private: bool disposed;
			private: UInt64 simulateUntil;
			// The number of processed events
			private: UInt64 numberOfProcessedEvents;
			private: bool started;
			private: UInt32 channelCapacity;
			private: Double totalSimulationTime;
			private: UInt64 localSimulationTime;
			//private: LogicalProcess **objects;
			private: vector<LogicalProcess *> *objects;
			// This is a handle to the set of pending events.
			private: pri_queue *pq;
			private: OnPartitioningCompletion on_partitioning_completion_callback;
			private: Object *on_partitioning_completion_state;			//private: EndOfSimulation end_of_simulation_callback;
			private: EventInterface *previous_event;
#if defined __space_time_graph_enabled__
			private: SpaceTimeGraph *stg;
			private: UInt32 last_event_id;
			private: bool prepared;
#endif
			/*private: class EventInformation
			{
                public: Int64 sendTime;
                public: Int64 recvTime;
                public: Int64 counter;
                public: UInt32 sender;
                public: UInt32 receiver;
                public: UInt32 id;
                public: EventInformation() : id(-1) {}
                public: inline friend bool operator == (const EventInformation &e1, const EventInformation &e2)
                {
                    if(e1.sendTime == e2.sendTime && e1.recvTime == e2.recvTime && e1.sender == e2.sender && e1.receiver == e2.receiver)
                        return true;
                    else
                        return false;
                }
                public: inline friend ::std::ostream &operator<<(::std::ostream &stream, const EventInformation &e)
                {
                    stream << "(sender,receiver,sendTime,recvTime)=(" << e.sender << "," << e.receiver << "," << e.sendTime << "," << e.recvTime << ")";
                    return stream;
                }
			};
			private: std::unordered_map<Int64, std::vector<EventInformation *>* > sendList;
			private: std::unordered_map<Int64, std::vector<EventInformation *>* > receiveList;*/
            //private: bool disposed;
			/*---------------------methods----------------*/
            // SequentialSimulationManager Class constructor.
			public: SequentialSimulationManager(UInt32 channelCapacity);
			// SequentialSimulationManager Class destructor.
			public: ~SequentialSimulationManager();
			private: void Dispose();

			// Overridden methods
			public: void RegisterLogicalProcess(LogicalProcess *lp);
			private: void RegisterOnPartitioningCompletionCallback(OnPartitioningCompletion callback, Object *state);
			public: void SendEvent(EventInterface *e);
			public: EventInterface *GetNextEvent(LogicalProcess *lp);
			public: EventInterface *PeekNextEvent(LogicalProcess *lp);

			public: inline UInt64 GetLocalSimulationTime(LogicalProcess *lp)
			{
				//UInt64 simeTime = GetSimulationTime();
				//if(simeTime == -1)
				//	return 0;
				//return simeTime;
				return localSimulationTime;
			}
			public: inline UInt32 GetNumberOfLogicalProcesses();
			public: void Run(UInt64 simulateUntil);
			public: void WaitOnManagerCompletion();
			public: LogicalProcess *GetLogicalProcessById(UInt32 id);
			// Is called when deleting the items of a priority queue.
			private: static void OnDisposePriorityQueue(EventInterface *e);
			public: inline UInt32 GetClusterCount() { return 1; }
			public: inline SimulationManagerType GetSimulationManagerType() { return Sequential; }
			//public: void RegisterEndOfSimulationCallback(EndOfSimulation callback);
            /// These methods must be implemented by the programmer.
            /// Other methods
		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

