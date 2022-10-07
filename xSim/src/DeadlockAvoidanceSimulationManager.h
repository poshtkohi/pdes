/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_DeadlockAvoidanceSimulationManager_h__
#define __Parvicursor_xSim_DeadlockAvoidanceSimulationManager_h__

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

#include "SimulationManagerType.h"
#include "SimulationManagerInterface.h"
#include "LogicalProcess.h"
#include "EventInterface.h"
#include "DeadlockAvoidancePriorityQueue.h"
#include "Partitioner.h"
#include "PartitionInfo.h"
#include "Vector.h"
#include "priority_queue.h"
//#include "Cluster.h"

using namespace System;
using namespace System::Threading;
using namespace LockFree;
using namespace Parvicursor::Profiler;
using namespace Parvicursor::xSim::psc;
using namespace Parvicursor::PS2;

#include <iostream>
#include <vector>

using namespace std;

extern Mutex *mtx;

#if !defined WIN32 || WIN64
#include <pthread.h>
#endif

#include <limits.h>

extern LockFreeCounter global_termination_detection_counter;
//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		//class DeadlockAvoidancePriorityQueue;

		class DeadlockAvoidanceSimulationManager : public SimulationManagerInterface
		{
		    // Forward declarations.
			//class DeadlockAvoidanceSimulationManager;
			//class DeadlockAvoidancePriorityQueue;

            private: class Cluster : public Object
            {
                friend class DeadlockAvoidanceSimulationManager;
				friend class DeadlockAvoidancePriorityQueue;

				private: UInt64 nullMessageCounter;
				private: UInt64 tNull, tLastNull, minNormal, minNull;
                private: UInt32 destinationClustersCount;
                private: UInt32 channelCapacity;
                private: UInt32 numberOfClusters;
                private: UInt64 numberOfProcessedEvents; ///
                private: UInt32 clusterID;
                private: bool terminated;
                public: bool terminationHasRequested; ///
                private: UInt64 localTime;
                public: UInt32 workingInputChannelCount; ///
				public: UInt32 blockingCount; ///
				private: pri_queue *localQueue;
				private: Vector<DeadlockAvoidancePriorityQueue *> inputQueues;
				private: OnBlockInformation onBlock;
				private: UInt32 terminationCounter;
				private: LockFreeCounter eventCounter;
				private: bool disposed;
				// Stores destination clusters. The integer key indicates the destination cluster ID and the value stores the destionation input channel.
				// private: map<Int32, DeadlockAvoidancePriorityQueue *> destionationClusters;
				private: Vector<DeadlockAvoidancePriorityQueue *> destinationClusters;
				private: Vector<DeadlockAvoidancePriorityQueue *> _destinationClusters;
                private: DeadlockAvoidancePriorityQueue *blockedPriorityQueue;
                private: SimulationManagerInterface *manager;
                // Cluster Class constructor.
                public: Cluster(UInt32 ClusterID, UInt32 numberOfClusters, UInt32 channelCapacity, SimulationManagerInterface *managerInstance);
                // Cluster Class destructor.
                public: ~Cluster();
                // Adds a new input channel and returns its reference to be used by the simulation kernel.
                public: inline DeadlockAvoidancePriorityQueue *AddInputChannel();
                // Gets the count of input channels.
                public: inline UInt32 GetInputChannelCount();
                // Gets the input channel specified by index.
                public: inline DeadlockAvoidancePriorityQueue *GetInputChannelByIndex(UInt32 index);
                // Checks whether this cluster has connectivity with the destination cluster specified by ID.
                public: inline bool ContainsDestinationCluster(UInt32 destionationClusterID);
                // Adds a new destination cluster to this cluster.
                public: inline void AddDestionationCluster(UInt32 destionationClusterID, DeadlockAvoidancePriorityQueue *destinationChannel);
                // Gets the number of connected destination clusters to this cluster.
                public: inline UInt32 GetDestinationClusterCount();
                // Gets the destination channel.
                public: inline DeadlockAvoidancePriorityQueue *GetDestinationClusterChannelByID(UInt32 destionationClusterID);
                // Gets the local priority queue.
                public: inline pri_queue *GetLocalPriorityQueue();
                // Gets the ID of this instance.
                public: inline UInt32 GetClusterID();
                // Gets the local time of this instance.
                public: inline UInt64 GetLocaltime();
                // Sets the local time of this instance to a new value.
                public: inline void SetLocaltime(UInt64 newTime);
                public: inline void RemoveInputChannelByIndex(UInt32 index);
                public: inline bool GetTerminated();
                public: inline void SetTerminated(bool val);
            };
            /*---------------------fields-----------------*/
			friend class DeadlockAvoidancePriorityQueue;

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
			private: Thread *controller;
			private: bool controllerThreadTerminationRequested;
			private: struct timeval stop;
			private: OnPartitioningCompletion on_partitioning_completion_callback;
			private: Object *on_partitioning_completion_state;
            private: bool disposed;
			/*---------------------methods----------------*/
            // DeadlockAvoidanceSimulationManager Class constructor.
			public: DeadlockAvoidanceSimulationManager(UInt32 numOfLogicalProcessors, UInt32 channelCapacity, Partitioner *partitioner);
			// DeadlockAvoidanceSimulationManager Class destructor.
			public: ~DeadlockAvoidanceSimulationManager();
			private: void Dispose();

			/// Overridden methods
			public: void RegisterLogicalProcess(LogicalProcess *lp);
			public: void RegisterOnPartitioningCompletionCallback(OnPartitioningCompletion callback, Object *state);
			public: inline void SendEvent(EventInterface *e)
            {
                UInt32 srcClusterID = LogicalProcess::GetLogicalProcessById(e->GetSenderReference())->GetOwnerClusterID();
                UInt32 destClusterID = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference())->GetOwnerClusterID();

				Cluster *srcCluster = clusters[srcClusterID];
				Cluster *destCluster = clusters[destClusterID];

				destCluster->eventCounter.Increment();

                if(srcClusterID == destClusterID)
                    priq_push(srcCluster->GetLocalPriorityQueue(), e, e->GetReceiveTime());
				else
                    srcCluster->GetDestinationClusterChannelByID(destClusterID)->Enqueue(e);

				//if(counter == 0)
				//	global_termination_detection_counter.Increment();

				//std::cout << "SendEvent() destCluster " << destClusterID << " counter " << destCluster->eventCounter.get_Count() << endl;

				//global_termination_detection_counter.Increment();
            }

			public: inline EventInterface *GetNextEvent(LogicalProcess *lp)
            {
                EventInterface *e = lp->e;

				Cluster *ownerCluster = clusters[lp->GetOwnerClusterID()];
                //if(e != null)
                ownerCluster->numberOfProcessedEvents++;

				//UInt32 srcClusterID = LogicalProcess::GetLogicalProcessById(e->GetSenderReference())->GetOwnerClusterID();
				//UInt32 destClusterID = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference())->GetOwnerClusterID();

				//Cluster *srcCluster = clusters[srcClusterID];
				//Cluster *destCluster = clusters[destClusterID];

				//destCluster->eventCounter.Decrement();

				//std::cout << "GetNextEvent() destCluster " << destClusterID << " counter " << destCluster->eventCounter.get_Count() << endl;

                return e;
            }
            public: EventInterface *PeekNextEvent(LogicalProcess *lp) {return null;}

			public: inline UInt64 GetLocalSimulationTime(LogicalProcess *lp)
			{
				//return clusters[lp->GetOwnerClusterID()]->GetLocaltime();
				return lp->localTime;
			}
			public: inline UInt32 GetNumberOfLogicalProcesses();
			public: void Run(UInt64 simulateUntil);
			public: void WaitOnManagerCompletion();
			public: inline LogicalProcess *GetLogicalProcessById(UInt32 id);
			public: inline UInt64 GetSimulationTime();
			public: inline void SetSimulationTime(UInt64 newTime);

            /// These methods must be implemented by the programmer.
            /// Other methods
			// Initializes the environment for parallel conservative simulation.
			private: void PrepareForParallelSimulation();
			// This method is executed as the function pointer by each worker thread.
			private: void Worker();
			private: static void *Wrapper_To_Call_Worker(void *pt2Object);
			// The following methods are used for global termination detection the conservative simulation.
			private: void Controller();
			private: static void *Wrapper_To_Call_Controller(void *pt2Object);
			private: template < typename T > static string to_string(const T &n)
			{
				std::ostringstream stm ;
				stm << n ;
				return stm.str() ;
			}
			// Is called when deleting the items of a priority queue.
			private: static void OnDisposePriorityQueue(EventInterface *e);
			public: inline UInt32 GetClusterCount() { return clusters.Size(); }
			private: inline void SendNullMassage(UInt32 currentCluster);
			private: static void OnBlock(Object *state, UInt32 OwnerClusterID);
			public: inline SimulationManagerType GetSimulationManagerType() { return DeadlockAvoidance; }
		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

