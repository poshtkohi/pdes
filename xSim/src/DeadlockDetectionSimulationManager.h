/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_DeadlockDetectionSimulationManager_h__
#define __Parvicursor_xSim_DeadlockDetectionSimulationManager_h__

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
#include <System.IO/IOException/IOException.h>

#include <../Samples/LockFree/LockFree/LockFreeCounter/LockFreeCounter.h>

#include "SimulationManagerType.h"
#include "SimulationManagerInterface.h"
#include "LogicalProcess.h"
#include "EventInterface.h"
#include "DeadlockDetectionPriorityQueue.h"
#include "Partitioner.h"
#include "PartitionInfo.h"
#include "Vector.h"
#include "priority_queue_lc.h"
#include "PdesTime.h"

using namespace System;
using namespace System::IO;
using namespace System::Threading;
using namespace LockFree;
using namespace Parvicursor::Profiler;
using namespace Parvicursor::xSim::psc;
using namespace Parvicursor::PS2;

#include <iostream>
#include <vector>

using namespace std;

extern Mutex *mtx;
//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		//class DeadlockDetectionPriorityQueue;

		class DeadlockDetectionSimulationManager : public SimulationManagerInterface
		{
			// Forward declarations.
			//class DeadlockDetectionSimulationManager;
			//class DeadlockDetectionPriorityQueue;

            private: class Cluster : public Object
            {
                friend class DeadlockDetectionSimulationManager;

				private: UInt32 nonLocalSentMessages;
				private: bool blocked;
				private: bool disposed;
                private: UInt32 destinationClustersCount;
                private: UInt32 channelCapacity;
                private: UInt32 numberOfClusters;
                private: UInt64 numberOfProcessedEvents; ///
                private: PdesTime mimimumTimestamp;
                private: UInt32 clusterID;
                private: bool terminated;
                public: bool terminationHasRequested; ///
                private: UInt64 localTime;
                public: UInt32 workingInputChannelCount; ///
                public: UInt32 blockingCount; ///
                private: UInt64 latestSendTime;
				private: pri_queue_lc *localQueue;
				private: Vector<DeadlockDetectionPriorityQueue *> inputQueues;
				private: DeadlockDetectionPriorityQueue *mimimumTimestampQueue;
				//private: Vector<pair<EventInterface *, Int32> > *messages;
				//private: Int32 messagesIndex;
				private: EventInterface *previous_event;
				private: UInt64 nullMessageCounter;
				private: UInt64 lc_time;
				// Stores destination clusters. The integer key indicates the destination cluster ID and the value stores the destionation input channel.
				// private: map<Int32, DeadlockDetectionPriorityQueue *> destionationClusters;
				private: Vector<DeadlockDetectionPriorityQueue *> destinationClusters;
				private: Vector<DeadlockDetectionPriorityQueue *> _destinationClusters;
                private: DeadlockDetectionPriorityQueue *blockedPriorityQueue;
                private: SimulationManagerInterface *manager;
                // Cluster Class constructor.
                public: Cluster(UInt32 ClusterID, UInt32 numberOfClusters, UInt32 channelCapacity, SimulationManagerInterface *managerInstance);
                // Cluster Class destructor.
                public: ~Cluster();
                // Adds a new input channel and returns its reference to be used by the simulation kernel.
                public: inline DeadlockDetectionPriorityQueue *AddInputChannel();
                // Gets the count of input channels.
                public: inline UInt32 GetInputChannelCount();
                // Gets the input channel specified by index.
                public: inline DeadlockDetectionPriorityQueue *GetInputChannelByIndex(UInt32 index);
                // Checks whether this cluster has connectivity with the destination cluster specified by ID.
                public: inline bool ContainsDestinationCluster(UInt32 destionationClusterID);
                // Adds a new destination cluster to this cluster.
                public: inline void AddDestionationCluster(UInt32 destionationClusterID, DeadlockDetectionPriorityQueue *destinationChannel);
                // Gets the number of connected destination clusters to this cluster.
                public: inline UInt32 GetDestinationClusterCount();
                // Gets the destination channel.
                public: inline DeadlockDetectionPriorityQueue *GetDestinationClusterChannelByID(UInt32 destionationClusterID);
                // Gets the local priority queue.
                public: inline pri_queue_lc *GetLocalPriorityQueue();
                // Sets the minimum timestamp during the deadlock recovery algorithm used by controller thread.
                public: inline void SetMinimumTimestamp(const PdesTime &newVal);
                // Gets the minimum timestamp during the deadlock recovery algorithm used by controller thread.
                public: inline PdesTime GetMinimumTimestamp();
                // Sets the minimum timestamp queue during the deadlock recovery algorithm used by controller thread.
                public: inline void SetMinimumTimestampQueue(DeadlockDetectionPriorityQueue *inputQueue);
                // Gets the minimum timestamp queue during the deadlock recovery algorithm used by controller thread.
                public: inline DeadlockDetectionPriorityQueue *GetMinimumTimestampQueue();
                // Gets the ID of this instance.
                public: inline UInt32 GetClusterID();
                // Gets the local time of this instance.
                public: inline UInt64 GetLocaltime();
                // Sets the local time of this instance to a new value.
                public: inline void SetLocaltime(UInt64 newTime);
                public: inline void RemoveInputChannelByIndex(UInt32 index);
                public: inline bool GetTerminated();
                public: inline void SetTerminated(bool val);
                public: inline void SetLatestSendTime(UInt64 newTime);
                public: inline UInt64 GetLatestSendTime();
                public: inline DeadlockDetectionPriorityQueue *GetBlockedPriorityQueue();
                public: inline void SetBlockedPriorityQueue(DeadlockDetectionPriorityQueue *newQueue);
                public: inline void SetBlocked(bool val);
                public: inline bool GetBlocked();
            };
            /*---------------------fields-----------------*/
			private: Vector<LogicalProcess *> *objects;
			private: Vector<Thread *> workers;
			private: Vector<Cluster *> clusters;
			// This is a handle to the set of pending events.
			private: UInt64 simulateUntil;
			// The current simulation time.
			private: UInt64 simulationTime;
			// The number of processed events
			private: UInt64 numberOfProcessedEvents;
			private: UInt64 totalSimulationTime;
			private: UInt32 channelCapacity;
			//private: LockFreeCounter threadCounter;
			private: UInt32 threadCounter;
			private: Mutex *mutex_threadCounter;
			private: bool hasPreparedForParallelSimulation;
			private: pthread_spinlock_t lock;
			private: int pshared;
			private: bool started;
			private: Partitioner *partitioner;
			private: PartitionInfo *partitions;
			private: UInt32 numOfLogicalProcessors;
			// The following variables are used in order to synchronizing the main thread and the worker threads.
			private: Mutex *mutex_completion;
			private: ConditionVariable *waitcv_completion;
			private: UInt32 completedThreadCount;
			// The following variables are used in order to synchronizing among different execution phases of the worker threads.
			private: Mutex *mutex_simulation_phases;
			private: ConditionVariable *waitcv_simulation_phases;
			private: bool simulation_phases_met;
			private: UInt32 completedThreadCount_simulation_phases;
			private: struct timeval start;
			private: struct timeval stop;
			// The following variables are used for Deadlock Detection and Recovery in the conservative synchronization protocol.
			private: Thread *controller; // Implements a termination mechanism for controller thread.
			//private: Mutex *mutex_controller;
			//private: ConditionVariable *waitcv_controller;
			//private: LockFreeCounter deadlock_counter;
			//private: UInt64 deadlock_counter;
			//private: Mutex *mutex_deadlock_counter;
			private: bool controllerThreadTerminationRequested;
			//private: bool met;
			private: UInt64 numberOfDeadlocks;
			//private: EventInterface **clusterEvents;
			private: OnPartitioningCompletion on_partitioning_completion_callback;
			private: Object *on_partitioning_completion_state;
			private: LockFreeCounter null_message_counter;
            private: bool disposed;
			/*---------------------methods----------------*/
            // DeadlockDetectionSimulationManager Class constructor.
			public: DeadlockDetectionSimulationManager(UInt32 numOfLogicalProcessors, UInt32 channelCapacity, Partitioner *partitioner);
			// DeadlockDetectionSimulationManager Class destructor.
			public: ~DeadlockDetectionSimulationManager();
			private: void Dispose();

			/// Overridden methods
			public: void RegisterLogicalProcess(LogicalProcess *lp);
			public: void RegisterOnPartitioningCompletionCallback(OnPartitioningCompletion callback, Object *state);

            public: EventInterface *GetNextEvent(LogicalProcess *lp);
            public: EventInterface *PeekNextEvent(LogicalProcess *lp);
            public: void SendEvent(EventInterface *e);

			public: inline UInt64 GetLocalSimulationTime(LogicalProcess *lp)
			{
				//return clusters[lp->GetOwnerClusterID()]->GetLocaltime();
				return lp->localTime;
			}
			public: inline UInt64 GetNextEventTime(LogicalProcess *lp);
			public: inline UInt32 GetNumberOfLogicalProcesses();
			public: void Run(UInt64 simulateUntil);
			public: void WaitOnManagerCompletion();
			public: inline LogicalProcess *GetLogicalProcessById(UInt32 id);
			public: inline UInt64 GetSimulationTime();
			public: inline void SetSimulationTime(UInt64 newTime);
			public: inline void SetBlockedPriorityQueue(UInt32 ClusterID, Object *pq)
			{
                if(pq == null)
                    clusters[ClusterID]->SetBlocked(false);
                else
                    clusters[ClusterID]->SetBlockedPriorityQueue((DeadlockDetectionPriorityQueue *)pq);
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
			private: static void AlarmShape(Int32 rows);
			private: inline PdesTime FindMinimumTimestampPerCluster(UInt32 ClusterID);
			private: inline void DeadlockRecoveryBarrierPhase(/*string who*/);
			private: template < typename T > static string to_string(const T &n)
			{
				std::ostringstream stm ;
				stm << n ;
				return stm.str() ;
			}
			// Is called when deleting the items of a priority queue.
			private: static void OnDisposePriorityQueue(EventInterface *e);
			public: inline UInt32 GetClusterCount() { return clusters.Size(); }
			public: SimulationManagerType GetSimulationManagerType() { return DeadlockDetection; }
			private: void GenerateDotGraph(const String &filename);
			//private: inline void SendNullMassage(UInt32 currentCluster);
			//private: inline void HandleNullMassage(EventInterface *e, UInt32 currentCluster);

		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

