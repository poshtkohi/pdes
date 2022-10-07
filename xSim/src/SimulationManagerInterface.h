/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/


#ifndef __Parvicursor_xSim_SimulationManagerInterface_h__
#define __Parvicursor_xSim_SimulationManagerInterface_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>

#include "SimulationManagerType.h"

using namespace System;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		typedef void (*OnPartitioningCompletion)(Object *state);

		// forward declarations
		class EventInterface;
		class LogicalProcess;

		interface class SimulationManagerInterface : public Object
		{
            /*---------------------fields-----------------*/
            private: UInt64 localTime;
			/*---------------------methods----------------*/
			// Registers a new logical process instance within the simulation manager.
			public: virtual void RegisterLogicalProcess(LogicalProcess *lp) = 0;
			// Register a callback that can be called at the end of simulation by the manager instance.
			//public: virtual void RegisterEndOfSimulationCallback(EndOfSimulation callback) = 0;
			// Sends an event with the information inside the e variable.
			public: virtual void SendEvent(EventInterface *e) = 0;
			// Gets a pointer to the next event at the current simulation time and removes it from the manager.
			public: inline virtual EventInterface *GetNextEvent(LogicalProcess *lp) = 0;
			// Peeks a pointer to the next event at the current simulation time and does not remove it from the manager.
			//public: inline virtual EventInterface *PeekNextEvent(LogicalProcess *lp) = 0;
			public: inline virtual EventInterface *PeekNextEvent(LogicalProcess *lp) = 0;//{ return null; };
			// Gets the time of the next event.
			//public: virtual UInt64 GetNextEventTime(LogicalProcess *lp) = 0;
			// Gets the number of logical processor objects registered with the simulation manager.
			public: virtual UInt32 GetNumberOfLogicalProcesses() = 0;
			// Runs the simulation by the simulation manager instance.
			public: virtual void Run(UInt64 simulateUntil) = 0;
			// Waits on the completion of the whole simulation execution.
			public: virtual void WaitOnManagerCompletion() = 0;
			// Gets the registered and allocated logical process instance specified by id.
			public: virtual LogicalProcess *GetLogicalProcessById(UInt32 id) = 0;
			// Gets the the simulation time.
			public: inline UInt64 GetSimulationTime() { return localTime; }
			// Sets the the simulation time to a new one.
			public: inline void SetSimulationTime(UInt64 newTime) { localTime = newTime; }
			public: inline virtual void SetBlockedPriorityQueue(UInt32 ClusterID, Object *pq) { }
			// Gets the the local simulation time of a logical process.
			public: virtual UInt64 GetLocalSimulationTime(LogicalProcess *lp) = 0;
			// Gets the number of clusters (partitions).
			public: virtual UInt32 GetClusterCount() = 0;
			public: virtual void RegisterOnPartitioningCompletionCallback(OnPartitioningCompletion callback, Object *state) = 0;
			// Gets the type of the simulation manager runtime.
			public: virtual SimulationManagerType GetSimulationManagerType() = 0;
			protected: void GenerateLpDotGraph(const String& filename);
		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

