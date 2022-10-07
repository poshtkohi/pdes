/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_TimeWarpSimulationManager_h__
#define __Parvicursor_xSim_TimeWarpSimulationManager_h__

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
#include <System.Threading/Mutex/Mutex.h>
#include <System.Threading/ConditionVariable/ConditionVariable.h>
#include <System.Threading/ThreadStateException/ThreadStateException.h>
#include <System/Environment/Environment.h>
#include <Parvicursor/Profiler/ResourceProfiler.h>

#include <../Samples/LockFree/LockFree/LockFreeCounter/LockFreeCounter.h>

#include "SimulationManagerInterface.h"
#include "LogicalProcess.h"
#include "EventInterface.h"
#include "Partitioner.h"
#include "PartitionInfo.h"
#include "Vector.h"
#include "ProcessingElement.h"
#include "SortedList.h"
#include "State.h"

using namespace System;
using namespace System::Threading;
using namespace LockFree;
using namespace Parvicursor::Profiler;
using namespace Parvicursor::xSim::psc;
using namespace Parvicursor::xSim::Collections;

#include <iostream>
#include <vector>

using namespace std;
//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		// Forward declarations.
		//class ProcessingElement;
		class TimeWarpSimulationManager : public SimulationManagerInterface
		{
            /*---------------------fields-----------------*/
			private: Vector<LogicalProcess *> *objects;
			private: Vector<Thread *> workers;
			private: Vector<ProcessingElement *> processingElements;
			private: Partitioner *partitioner;
			private: PartitionInfo *partitions;
			private: UInt32 numOfLogicalProcessors;
			private: UInt64 simulateUntil;
			// The current simulation time.
			private: UInt64 simulationTime;
			// The number of processed events
			private: UInt64 numberOfProcessedEvents;
			private: UInt64 totalSimulationTime;
			private: UInt32 channelCapacity;
			private: LockFree::LockFreeCounter threadCounter;
			private: bool hasPreparedForParallelSimulation;
			private: Thread *controller;
			// The following variables are used in order to synchronizing the main thread and the worker threads.
			private: Mutex *mutex_completion;
			private: ConditionVariable *waitcv_completion;
			private: UInt32 completedThreadCount;
			// The following variables are used in order to synchronizing among different execution phases of the worker threads.
			private: Mutex *mutex_simulation_phases;
			private: ConditionVariable *waitcv_simulation_phases;
			private: UInt32 completedThreadCount_simulation_phases;
			private: bool started;
			private: struct timeval start;
			private: struct timeval stop;
			private: bool controllerThreadTerminationRequested;
			private: OnPartitioningCompletion on_partitioning_completion_callback;
			private: Object *on_partitioning_completion_state;
            private: bool disposed;
			/*---------------------methods----------------*/
            // TimeWarpSimulationManager Class constructor.
			public: TimeWarpSimulationManager(UInt32 numOfLogicalProcessors, Partitioner *partitioner);
			// TimeWarpSimulationManager Class destructor.
			public: ~TimeWarpSimulationManager();
			private: void Dispose();

			/// Overridden methods
			public: void RegisterLogicalProcess(LogicalProcess *lp);
			private: void RegisterOnPartitioningCompletionCallback(OnPartitioningCompletion callback, Object *state);
			public: inline void SendEvent(EventInterface *e);
			public: inline EventInterface *GetNextEvent(LogicalProcess *lp);

            public: inline EventInterface *PeekNextEvent(LogicalProcess *lp);
			public: inline UInt64 GetNextEventTime(LogicalProcess *lp);
			public: inline UInt32 GetNumberOfLogicalProcesses();
			public: void Run(UInt64 simulateUntil);
			public: void WaitOnManagerCompletion();
			public: inline LogicalProcess *GetLogicalProcessById(UInt32 id);
			public: inline UInt64 GetSimulationTime();
			public: inline void SetSimulationTime(UInt64 newTime);
			public: inline UInt64 GetLocalSimulationTime(LogicalProcess *lp)
			{
				//return clusters[lp->GetOwnerClusterID()]->GetLocaltime();
				//return lp->localTime;
				//return clusters[lp->GetOwnerClusterID()]->GetLocaltime();
				return processingElements[lp->GetOwnerClusterID()]->localVirtualTime;
			}

            /// These methods must be implemented by the programmer.
            /// Other methods
			// Initializes the environment for parallel conservative simulation.
			private: void PrepareForParallelSimulation();
			// This method is executed as the function pointer by each worker thread.
			private: void Worker();
			private: static void *Wrapper_To_Call_Worker(void *pt2Object);
			// The following methods are used for deadlock detection and recovery during the conservative simulation.
			private: void Controller();
			private: static void *Wrapper_To_Call_Controller(void *pt2Object);
			private: inline void SimulationPhasesBarrier();
			private: template < typename T > static string to_string(const T &n)
			{
				std::ostringstream stm ;
				stm << n ;
				return stm.str() ;
			}
		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

