/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_LogicalProcess_h__
#define __Parvicursor_xSim_LogicalProcess_h__

// Enables printing the debugging information across the xSim framework.
//#define __Parvicursor_xSim_Debug_Enable__ true

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/DateTime/DateTime.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>
#include <System/ArgumentException/ArgumentException.h>
#include <System/InvalidOperationException/InvalidOperationException.h>
#include <System.Threading/Mutex/Mutex.h>

#include "EventInterface.h"
#include "SortedList.h"
#include "List.h"
#include "SimulationManagerInterface.h"
//#include "SequentialSimulationManager.h"
#include "State.h"
#include "Vector.h"
//#include "DeadlockAvoidancePriorityQueue.h"
//#include "DeadlockAvoidanceSimulationManager.h"

using namespace System;
using namespace System::Threading;
using namespace Parvicursor::xSim;
using namespace Parvicursor::xSim::Collections;
using namespace Parvicursor::xSim::psc;


#include <iostream>
#include <vector>
#include <map>

using namespace std;

extern Vector<LogicalProcess *> *logicalProcesses;

//**************************************************************************************************************/

namespace Parvicursor
{
	namespace xSim
	{
		// Forward declarations.
		class SequentialSimulationManager;
		class DeadlockAvoidanceSimulationManager;
		class TimeWarpSimulationManager;
		class Partitioner;
		class RoundRobinPartitioner;
		class DepthFirstSearchPartitioner;
		class DeadlockAvoidancePriorityQueue;
		class DeadlockDetectionSimulationManager;
		class ProcessingElement;
		class SpaceTimeGraph;
		class EventInterface;

		//----------------------------------------------------
		class LogicalProcess : public Object
		{
			friend class SequentialSimulationManager;
			friend class DeadlockAvoidanceSimulationManager;
			friend class DeadlockDetectionSimulationManager;
			friend class TimeWarpSimulationManager;
			friend class Partitioner;
			friend class RoundRobinPartitioner;
			friend class DepthFirstSearchPartitioner;
			friend class ProcessingElement;
			friend class SpaceTimeGraph;
			friend class EventInterface;
			/*---------------------fields-----------------*/
			private: UInt64 localTime;
			private: UInt32 processId;
			private: Int32 processSpaceStateId;
			private: UInt64 lc_time;
			private: UInt32 clusterID;
			private: UInt32 index;
		 	private: EventInterface *e;
			private: SimulationManagerInterface *manager;
			private: bool isClockLogicalProcess;
			private: bool initialized;
			private: bool visited;
			public: bool marked;
			//private: bool disposed;
			private: Object *data;
			private: vector<Int32> outputLPs;
			private: std::map<Int32, Int32> outputLPsMap;
			private: vector<Int32> inputLPs;
			public: inline Object* GetData() { return data; }
			public: inline void SetData(Object* data) { this->data = data; }
			// The following variable is internally used by the simulation kernel (DeadlockAvoidanceSimulationManager).
			private: Object *pq;
			/*---------------------methods----------------*/
			// LogicalProcess Class constructor.
			public: LogicalProcess();
			// LogicalProcess Class destructor.
			public: ~LogicalProcess();
			// Adds a new output LP to this LP instance.
			public: void AddOutputLP(Int32 id);
			// Adds a new input LP to this LP instance.
			public: void AddInputLP(Int32 id);
			// Gets the ID of the current logical process instance.
			public: inline UInt32 GetID() { return processId; }
			// Gets the number of output logical processors to this instance.
			public: Int32 GetOutputLpCount();
			// Gets the number of output logical processors to this instance.
			public: Int32 GetInputLpCount();
			// Gets the ID of output logical processor specified by index.
			public: Int32 GetOutputLpId(Int32 index);
			// Gets the ID of input logical processor specified by index.
			public: Int32 GetInputLpId(Int32 index);
			private: void SetInitialized(bool val) { initialized = val; }
			private: bool GetInitialized() { return initialized; }
			// Sends an event with the information inside the e variable. The event must be an instance of the EventInterface class.
			public: inline void SendEvent(EventInterface *e) { manager->SendEvent(e); }
			// Gets a pointer to the next event at the current simulation time. The returned event must be an instance of the EventInterface class.
			public: inline EventInterface *GetNextEvent() {  return manager->GetNextEvent(this); }
			// Peeks a pointer to the next event at the current simulation time. The returned event must be an instance of the EventInterface class.
			public: inline EventInterface *PeekNextEvent() {  return manager->PeekNextEvent(this); }
			// Request the simulation manager to terminate the execution.
			public:	void Terminate();
            // Peeks a pointer to the next event at the current simulation time and does not remove it from the manager. The returned event must be an instance of the EventInterface class.
			//public: inline EventInterface *PeekNextEvent() { return manager->PeekNextEvent(this); }
			// Gets the time of the next event.
			//public: inline UInt64 GetNextEventTime() { return manager->GetNextEventTime(this); }
			private: void SetSimulationManager(SimulationManagerInterface *manager);
			// Gets the time of the next event.
			// Get the current local time for this logical process.
			public: inline UInt64 GetSimulationTime() { return localTime; /*return manager->GetLocalSimulationTime(this);*/  }
			public: inline void SetSimulationTime(UInt64 newTime) { localTime = newTime; }
			public: SimulationManagerType GetSimulationManagerType() { return manager->GetSimulationManagerType(); }
			// Searches the output LP map to see if the LP specified by id is connected to this logical process.
			public: inline bool ContainOutputLogicalProcess(Int32 id) { return outputLPsMap.count(id); }
			// Tells whether this simulation object has more events to process at the current simulation time.
			//public: bool HaveMoreEvents();
			// Sets the cluster ID in where this logical process resides. This function is internally used by the parallel xSim kernel.
			//private: void SetOwnerClusterID(Int32 id);
			public: void SetOwnerClusterID(UInt32 id);
			// Gets the cluster ID in where this logical process resides. This function is internally used by the parallel xSim kernel.
			//private: Int32 GetOwnerClusterID();
			public: inline UInt32 GetOwnerClusterID() { return clusterID; }

			/// These methods must be implemented by the programmer.
			// The kernel calls this method provided by the user. User must place the simulation code within the implementations of this method.
			// The Initialize() method is called by the simulation kernel at the beginning of the simulation.
			public: virtual void Initialize() = 0;
			// The Finalize() method is called by the simulation kernel at the end of the simulation.
			public: virtual void Finalize() = 0;
			// Calls to application to execute its code for one simulation cycle.
			public: virtual void ExecuteProcess() = 0;
			public: virtual const String &GetName() = 0;
			// Returns the state corresponding to the current simulation time.
			public: virtual State *GetState() = 0;
			// Calls the application to allocate state.
			public: virtual State *AllocateState() = 0;
			// Calls the application to deallocate state.
			public: virtual void DeallocateState(const State *state) = 0;
			// Calls the application to deallocate event.
			public: virtual void DeallocateEvent(const EventInterface *e) = 0;

			// The following method can optionally implemented by programmers.
			// The state variable can be used to pass arguments to the method.
			public: virtual void AxillaryMethod(Object *state);
			// This methods is called prior to Initialize().
			public: virtual void Elaborate();
			public: inline void SetIsClockLogicalProcess() { isClockLogicalProcess = true; }
			public: inline bool GetIsClockLogicalProcess() { return isClockLogicalProcess; }

			private: inline Object *GetPQ() { return pq; }
			/// Static methods
			// Gets the registered and allocated logical process instance specified by id.
			//public: LogicalProcess *GetLogicalProcessById(UInt64 id);
			public: inline static LogicalProcess *GetLogicalProcessById(UInt32 id) { return (*logicalProcesses)[id]; }
			public: inline static UInt32 GetNumberOfLogicalProcesses() { return logicalProcesses->Size(); }
			// Time Warp members
			private: class TimeWarpInfo
			{
				public: UInt64 localVirtualTime;
				public: SortedList<EventInterface *> *processedQueue;
				public: SortedList<EventInterface *> *outputQueue;
				public: List<EventInterface *> *negQueue;
				public: ProcessingElement *ownerPe;
				public: EventInterface *Ef;
				public: EventInterface *El;
				public: TimeWarpInfo(ProcessingElement *ownerPe, SortedList<EventInterface *>::OnDispose OnDisposeProcessedOrOutputQueue, List<EventInterface *>::OnRemove OnRemoveNegQueue)
				{
					localVirtualTime = 0;
					processedQueue = new SortedList<EventInterface *>(OnDisposeProcessedOrOutputQueue);
					outputQueue = new SortedList<EventInterface *>(OnDisposeProcessedOrOutputQueue);
					negQueue = new List<EventInterface *>(OnRemoveNegQueue);
					this->ownerPe = ownerPe;
				}
				public: ~TimeWarpInfo()
				{
					localVirtualTime = -1;
					delete processedQueue;
					processedQueue = null;
					delete outputQueue;
					outputQueue = null;
					delete negQueue;
					negQueue = null;
					//cout << "~TimeWarpInfo()" << endl;
				}
			};

			private: TimeWarpInfo *twi;
		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif


