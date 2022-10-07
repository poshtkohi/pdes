/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
    All rights reserved to Alireza Poshtkohi (c) 1999-2023.
    Email: arp@poshtkohi.info
    Website: http://www.poshtkohi.info
*/

#include "DeadlockDetectionSimulationManager.h"


// Stores the number of allocated logical processes.
extern UInt32 processCounter;
// Stores the allocated logical processes.
// We use an array to store the references to increase the access performance.
extern Vector<LogicalProcess *> *logicalProcesses;

//extern Mutex *mtx;

LockFreeCounter global_deadlock_counter = LockFreeCounter();
Mutex *global_mutex_controller = new Mutex();
ConditionVariable *global_waitcv_controller = new ConditionVariable(global_mutex_controller);
bool global_met = false;
//volatile int global_deadlock_counter_ = 0;
/*Int32 global_deadlock_counter;
pthread_spinlock_t global_counter_lock;
int pshared_;
__attribute__((constructor))
void global_counter_lock_constructor () {
if ( pthread_spin_init ( &global_counter_lock, pshared_ ) != 0 ) {
exit ( 1 );
}
}*/
// The following variables are used to synchronize the woken deadlocked clusters as a barrier.
UInt32 global_minimum_cluster_count = 0;
UInt32 completedThreadCount_deadlocked_clusters_phase = 0;
Mutex *deadlocked_clusters_mutex = new Mutex();
ConditionVariable *deadlocked_clusters_cv = new ConditionVariable(deadlocked_clusters_mutex);
bool deadlocked_clusters_met = false;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		DeadlockDetectionSimulationManager::Cluster::Cluster(UInt32 ClusterID, UInt32 numberOfClusters, UInt32 channelCapacity, SimulationManagerInterface *managerInstance)
		{
			nonLocalSentMessages = 0;
			localQueue = null;
			numberOfProcessedEvents = 0;
			localTime = 0;
			//messages = null;
			//messagesIndex = 0;
			previous_event = null;
			//if(ClusterID < 0)
			//	throw ArgumentOutOfRangeException("ClusterID", "ClusterID is less than zero");
			if(numberOfClusters == 0)
				throw ArgumentOutOfRangeException("numberOfClusters", "numberOfClusters is equal to zero");
			if(channelCapacity == 0)
				throw ArgumentOutOfRangeException("channelCapacity", "channelCapacity is equal to zero");
			if(managerInstance == null)
				throw ArgumentNullException("managerInstance", "managerInstance is a null reference");
			this->numberOfClusters = numberOfClusters;
			this->channelCapacity = channelCapacity;
			manager = managerInstance;
			localQueue = priq_new_lc(channelCapacity); ///
			destinationClusters.Reserve(numberOfClusters);
			for(register UInt32 i = 0 ; i < numberOfClusters ; i++)
				destinationClusters[i] = null;
			destinationClustersCount = 0;
			mimimumTimestamp = -1;
			mimimumTimestampQueue = null;
			clusterID = ClusterID;
			terminationHasRequested = false;
			terminated = false;
			workingInputChannelCount = 0;
			blockingCount = 0;
			blockedPriorityQueue = null;
			blocked = false;
			nullMessageCounter = 0;
			lc_time = 0;
			disposed = false;
		}
		//----------------------------------------------------
		DeadlockDetectionSimulationManager::Cluster::~Cluster()
		{
			//printf("~Cluster\n");//
			if(!disposed)
			{
				if(localQueue != null)
				{
					PdesTime pri;
					EventInterface *e;
					while( ( e = (EventInterface *)priq_pop_lc(localQueue, &pri) ) != null)
						DeadlockDetectionSimulationManager::OnDisposePriorityQueue(e);
					priq_delete_lc(localQueue);
				}

				for(register UInt32 i = 0 ; i < inputQueues.Size() ; i++)
					if(inputQueues[i] != null)
						delete inputQueues[i];

                if(previous_event != null)
				{
                    if(previous_event->deletable)
                    {
                        LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(previous_event->GetReceiverReference());
                        receiver->DeallocateEvent(previous_event);
                    }
                    previous_event = null;
				}

                /*if(messages != null)
                {
                    messages->Reset();
                    delete messages;
                }*/

				disposed = true;
			}
		}
		//----------------------------------------------------
		DeadlockDetectionPriorityQueue *DeadlockDetectionSimulationManager::Cluster::AddInputChannel()
		{
			if(disposed)
				throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			DeadlockDetectionPriorityQueue *inputQueue = new DeadlockDetectionPriorityQueue(channelCapacity, true, DeadlockDetectionSimulationManager::OnDisposePriorityQueue);
			inputQueue->ownerInputIndex = workingInputChannelCount;
			inputQueue->SetOwnerClusterID(GetClusterID());
			inputQueue->SetSimulationManager(manager);
			inputQueues.PushBack(inputQueue);
			workingInputChannelCount++;
			return inputQueue;
		}
		//----------------------------------------------------
		UInt32 DeadlockDetectionSimulationManager::Cluster::GetInputChannelCount()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");

			return inputQueues.Size();
		}
		//----------------------------------------------------
		DeadlockDetectionPriorityQueue *DeadlockDetectionSimulationManager::Cluster::GetInputChannelByIndex(UInt32 index)
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			//if(index < 0 || index > inputQueues.Size() - 1)
			//	throw ArgumentOutOfRangeException("index", "index must be greater than zero and be less than GetInputChannelCount()");
			return inputQueues[index];
		}
		//----------------------------------------------------
		bool DeadlockDetectionSimulationManager::Cluster::ContainsDestinationCluster(UInt32 destionationClusterID)
		{
			if(disposed)
				throw ObjectDisposedException("Cluster", "The Cluster has been disposed");

			//if(destionationClusterID < 0 || destionationClusterID > destinationClusters.Size() - 1)
			//	throw ArgumentOutOfRangeException("destionationClusterID", "destionationClusterID must be greater than or equal to zero and be less than numberOfClusters-1");

			if(destionationClusterID > destinationClusters.Size() - 1)
				throw ArgumentOutOfRangeException("destionationClusterID", "destionationClusterID must be greater than or equal to zero and be less than numberOfClusters-1");

			if(destinationClustersCount == 0)
				return false;

			if(destinationClusters[destionationClusterID] != null)
				return true;
			else
				return false;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Cluster::AddDestionationCluster(UInt32 destionationClusterID, DeadlockDetectionPriorityQueue *destinationChannel)
		{
			if(disposed)
				throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			if(destionationClusterID > destinationClusters.Size() - 1)
				throw ArgumentOutOfRangeException("destionationClusterID", "destionationClusterID must be greater than or equal to zero and be less than numberOfClusters-1");
			if(destinationChannel == null)
				throw ArgumentNullException("destinationChannel", "destinationChannel is null");
			destinationClusters[destionationClusterID] = destinationChannel;
			_destinationClusters.PushBack(destinationChannel);
			destinationClustersCount++;
		}
		//----------------------------------------------------
		UInt32 DeadlockDetectionSimulationManager::Cluster::GetDestinationClusterCount()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			return destinationClustersCount;
		}
		//----------------------------------------------------
		DeadlockDetectionPriorityQueue *DeadlockDetectionSimulationManager::Cluster::GetDestinationClusterChannelByID(UInt32 destionationClusterID)
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			//if(destionationClusterID < 0 || destionationClusterID > destinationClusters.Size() - 1)
			//	throw ArgumentOutOfRangeException("destionationClusterID", "destionationClusterID must be greater than or equal to zero and be less than numberOfClusters-1");
			return destinationClusters[destionationClusterID];
		}
		//----------------------------------------------------
		pri_queue_lc *DeadlockDetectionSimulationManager::Cluster::GetLocalPriorityQueue()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			return localQueue;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Cluster::SetMinimumTimestamp(const PdesTime &newVal)
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");

			mimimumTimestamp = newVal;
		}
		//----------------------------------------------------
		PdesTime DeadlockDetectionSimulationManager::Cluster::GetMinimumTimestamp()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			return mimimumTimestamp;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Cluster::SetMinimumTimestampQueue(DeadlockDetectionPriorityQueue *inputQueue)
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");

			// if inputQueue is null, it means the localQueue have the minimum timestamp
			mimimumTimestampQueue = inputQueue;
		}
		//----------------------------------------------------
		DeadlockDetectionPriorityQueue *DeadlockDetectionSimulationManager::Cluster::GetMinimumTimestampQueue()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			return mimimumTimestampQueue;
		}
		//----------------------------------------------------
		UInt32 DeadlockDetectionSimulationManager::Cluster::GetClusterID()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			return clusterID;
		}
		//----------------------------------------------------
		UInt64 DeadlockDetectionSimulationManager::Cluster::GetLocaltime()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			return localTime;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Cluster::SetLocaltime(UInt64 newTime)
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			localTime = newTime;
		}
		//----------------------------------------------------
		bool DeadlockDetectionSimulationManager::Cluster::GetTerminated()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");

			return terminated;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Cluster::SetTerminated(bool val)
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			terminated = val;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Cluster::RemoveInputChannelByIndex(UInt32 index)
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			//if(index < 0 || index > inputQueues.Size() - 1)
			//	throw ArgumentOutOfRangeException("index", "index must be greater than zero and be less than GetInputChannelCount()");
			//inputQueues[index] = null;
			inputQueues.RemoveAt(index);
			workingInputChannelCount--;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Cluster::SetLatestSendTime(UInt64 newTime)
		{
			latestSendTime = newTime;
		}
		//----------------------------------------------------
		UInt64 DeadlockDetectionSimulationManager::Cluster::GetLatestSendTime()
		{
			return latestSendTime;
		}
		//----------------------------------------------------
		DeadlockDetectionPriorityQueue *DeadlockDetectionSimulationManager::Cluster::GetBlockedPriorityQueue()
		{
			return blockedPriorityQueue;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Cluster::SetBlockedPriorityQueue(DeadlockDetectionPriorityQueue *newQueue)
		{
			blockedPriorityQueue = newQueue;
			SetBlocked(true);
			blockingCount++;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Cluster::SetBlocked(bool val)
		{
			blocked = val;
		}
		//----------------------------------------------------
		bool DeadlockDetectionSimulationManager::Cluster::GetBlocked()
		{
			return blocked;
		}
		//----------------------------------------------------
		DeadlockDetectionSimulationManager::DeadlockDetectionSimulationManager(UInt32 numOfLogicalProcessors, UInt32 channelCapacity, Partitioner *partitioner)
		{
			objects = null;
			partitions = null;
			controller = null;
			mutex_completion = null;
			waitcv_completion = null;
			mutex_simulation_phases = null;
			waitcv_simulation_phases = null;
			///waitcv_controller = null;
			///mutex_controller = null;
			///mutex_threadCounter = null;
			//mutex_deadlock_counter = null;
			if(numOfLogicalProcessors <= 0)
				throw ArgumentOutOfRangeException("numOfLogicalProcessors", "numOfLogicalProcessors is less than or equal to zero");
			if(channelCapacity <= 0)
				throw ArgumentOutOfRangeException("channelCapacity", "channelCapacity is less than or equal to zero");
			if(partitioner == null)
				throw ArgumentNullException("partitioner", "partitioner is null");
			this->numOfLogicalProcessors = numOfLogicalProcessors;
			this->partitioner = partitioner;
			this->channelCapacity = channelCapacity;
			objects = new Vector<LogicalProcess *>;
			numberOfProcessedEvents = 0;
			simulationTime = 0;
			started = false;
			hasPreparedForParallelSimulation = false;
			completedThreadCount = 0;
			completedThreadCount_simulation_phases = 0;
			controllerThreadTerminationRequested = false;
			//met = false;
			numberOfDeadlocks = 0;
			simulation_phases_met = false;
			on_partitioning_completion_callback = null;
			on_partitioning_completion_state = null;
			//clusterEvents = null;
			null_message_counter = LockFreeCounter(0);
			disposed = false;
		}
		//----------------------------------------------------
		DeadlockDetectionSimulationManager::~DeadlockDetectionSimulationManager()
		{
			//printf("~DeadlockDetectionSimulationManager\n");
			Dispose();
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Dispose()
		{
			if(!disposed)
			{
				if(hasPreparedForParallelSimulation)
				{
					for(register UInt32 i = 0 ; i < workers.Size() ; i++)
						if(workers[i] != null)
							delete workers[i];

					for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
						if(clusters[i] != null)
							delete clusters[i];
					/*if(clusterEvents != null)
					{
					Int32 numberOfClusterEvents = nearestPower(clusters.Size());
					for(register Int32 i = 0 ; i < numberOfClusterEvents ; i++)
					if(clusterEvents[i] != null)
					delete clusterEvents[i];
					delete clusterEvents;
					}*/
					if(controller != null)
						delete controller;

					if(partitions != null)
						delete partitions;

					if(mutex_completion != null)
						delete mutex_completion;
					if(waitcv_completion != null)
						delete waitcv_completion;

					if(mutex_simulation_phases != null)
						delete mutex_simulation_phases;
					if(waitcv_simulation_phases != null)
						delete waitcv_simulation_phases;

					/*if(mutex_controller != null)
					delete mutex_controller;
					if(waitcv_controller != null)
					delete waitcv_controller;
					if(mutex_threadCounter != null)
					delete mutex_threadCounter;
					if(mutex_deadlock_counter != null)
					delete mutex_deadlock_counter;*/
					pthread_spin_destroy(&lock);
				}

				if(objects != null)
					delete objects;

				disposed = true;
			}
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::RegisterLogicalProcess(LogicalProcess *lp)
		{
			if(disposed)
				throw ObjectDisposedException("DeadlockDetectionSimulationManager", "The DeadlockDetectionSimulationManager has been disposed");
			if(lp == null)
				throw ArgumentNullException("lp", "lp is null");
			objects->PushBack(lp);
			lp->SetSimulationManager(this);
		}
		//----------------------------------------------------
		/*void DeadlockDetectionSimulationManager::SendEvent(EventInterface *e)
		{
		//if(disposed)
		//	throw ObjectDisposedException("DeadlockDetectionSimulationManager", "The DeadlockDetectionSimulationManager has been disposed");
		//if(e == null)
		//	throw ArgumentNullException("e", "e is null");
		Int32 srcClusterID = e->GetSenderReference()->GetOwnerClusterID();
		Int32 destClusterID = e->GetReceiverReference()->GetOwnerClusterID();
		if(srcClusterID == destClusterID)
		clusters[srcClusterID]->GetLocalPriorityQueue()->EnqueueInternal(e, e->GetReceiveTime());
		else
		clusters[srcClusterID]->GetDestinationClusterChannelByID(destClusterID)->Enqueue(e, e->GetReceiveTime());
		//clusters[destClusterID]->GetDestinationClusterChannelByID(srcClusterID)->Enqueue(e, e->GetReceiveTime(), QueueNotEmptyNotificationCallback, this);
		}
		//----------------------------------------------------
		EventInterface *DeadlockDetectionSimulationManager::GetNextEvent(LogicalProcess *lp)
		{
		//if(disposed)
		//	throw ObjectDisposedException("DeadlockDetectionSimulationManager", "The DeadlockDetectionSimulationManager has been disposed");

		//if(lp == null)
		//	throw ArgumentNullException("lp", "lp is null");

		UInt64 timestamp;
		EventInterface *e = (EventInterface *)lp->pq->Dequeue(timestamp);

		if(e != null)
		clusters[lp->GetOwnerClusterID()]->numberOfProcessedEvents++;

		return e;
		}*/
		//----------------------------------------------------
		UInt64 DeadlockDetectionSimulationManager::GetNextEventTime(LogicalProcess *lp)
		{
			//if(disposed)
			//	throw ObjectDisposedException("DeadlockDetectionSimulationManager", "The DeadlockDetectionSimulationManager has been disposed");

			//if(lp == null)
			//	throw ArgumentNullException("lp", "lp is null");

			return lp->e->GetReceiveTime();
		}
		//----------------------------------------------------
		/*EventInterface *DeadlockDetectionSimulationManager::PeekNextEvent(LogicalProcess *lp)
		{
			//if(disposed)
			//	throw ObjectDisposedException("DeadlockDetectionSimulationManager", "The DeadlockDetectionSimulationManager has been disposed");

			//if(lp == null)
			//	throw ArgumentNullException("lp", "lp is null");

			//EventInterface *e = (EventInterface *)((DeadlockDetectionPriorityQueue *)lp->pq)->Peek();

			//return e;

			EventInterface *e = lp->e;
            return e;
		}*/
		//----------------------------------------------------
		UInt32 DeadlockDetectionSimulationManager::GetNumberOfLogicalProcesses()
		{
			//if(disposed)
			//	throw ObjectDisposedException("DeadlockDetectionSimulationManager", "The DeadlockDetectionSimulationManager has been disposed");

			return objects->Size();
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Run(UInt64 simulateUntil)
		{
			if(disposed)
				throw ObjectDisposedException("DeadlockDetectionSimulationManager", "The DeadlockDetectionSimulationManager has been disposed");

			if(simulateUntil <= 0)
				throw ArgumentOutOfRangeException("simulateUntil", "simulateUntil is less than or equal to zero.");

			if(GetNumberOfLogicalProcesses() == 0)
				throw ArgumentOutOfRangeException("GetNumberOfLogicalProcesses", "You must register at least one logical process instance");

			this->simulateUntil = simulateUntil;

			if(!started)
			{
				started = true;

				std::cout << "\n\t PS2 v2.2.1 (Many-Core Release)\n\t Conservative Protocol with Deadlock Detection & Recovery\n";
				std::cout << "\t Parvicursor Simulation System\n";
				std::cout << "\t Copyright (c) 2004-" << DateTime::get_Now().get_Year() << " by Shahed University,\n";
				std::cout << "\t ALL RIGHTS RESERVED\n\n" << std::endl;
				std::cout << "Initializing parallel simulation environment ..." << std::endl;

				PrepareForParallelSimulation();
			}
			else
				throw ThreadStateException("The simulation has already been started");
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::WaitOnManagerCompletion()
		{
			if(disposed)
				throw ObjectDisposedException("DeadlockDetectionSimulationManager", "The DeadlockDetectionSimulationManager has been disposed");

			if(!started)
				throw InvalidOperationException("Run() method must first be called");

			mutex_completion->Lock();
			while( completedThreadCount != clusters.Size() )
				waitcv_completion->Wait();
			mutex_completion->Unlock();
			std::cout << "Finalizing simulation ...\n";
			UInt64 numberOfProcessedEvents = 0;
			for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
				numberOfProcessedEvents += clusters[i]->numberOfProcessedEvents;
			double d1, d2;
			d1 = (double)start.tv_sec + 1e-6*((double)start.tv_usec);
			d2 = (double)stop.tv_sec + 1e-6*((double)stop.tv_usec);
			// return result in seconds
			Double totalSimulationTime = d2 - d1;

			UInt32 density1 = 0, density2 = 0;
			if(totalSimulationTime != 0)
				density1 = numberOfProcessedEvents / totalSimulationTime;

			std::cout << "Simulation completed (" << numberOfProcessedEvents << " events in "
				<< totalSimulationTime << " secs, "
				<< density1 << " events/sec, ";
			density1 = 0;
			if(totalSimulationTime != 0)
				density1 = numberOfDeadlocks / totalSimulationTime;

			//if(totalSimulationTime != 0)
			//	density2 = null_message_counter.get_Count() / totalSimulationTime;

            UInt64 totalNumOfSentNullMessages = 0;
			if(totalSimulationTime != 0)
			{
                for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
                    totalNumOfSentNullMessages += clusters[i]->nullMessageCounter;
                density2 = totalNumOfSentNullMessages / totalSimulationTime;
			}

			std::cout << numberOfDeadlocks << " deadlocks, " << density1 << " deadlocks/sec, " << totalNumOfSentNullMessages << " null-messages, " << density2 << " null-messages/sec executed on " << clusters.Size() << " logical processors with " << Environment::get_ProcessorCount() << " physical cores).\n" << std::endl;

			//for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
			//	std::cout << "Number of (blocking states, processed events) in Cluster " << i << " is (" << clusters[i]->blockingCount << ", " << clusters[i]->numberOfProcessedEvents << ")" << ".\n";

			std::cout << std::endl;

			usage u;
			ResourceProfiler::GetResourceUsage(&u);
			ResourceProfiler::PrintResourceUsage(&u);

			Dispose(); ///

			return ;
		}
		//----------------------------------------------------
		LogicalProcess *DeadlockDetectionSimulationManager::GetLogicalProcessById(UInt32 id)
		{
			//if(id < 0)
			//	throw ArgumentOutOfRangeException("id", "id is less than zero");

			return (*logicalProcesses)[id];
		}
		//----------------------------------------------------
		UInt64 DeadlockDetectionSimulationManager::GetSimulationTime()
		{
			if(disposed)
				throw ObjectDisposedException("DeadlockDetectionSimulationManager", "The DeadlockDetectionSimulationManager has been disposed");

			// Here, we must consider a lock-based or lock-free implementation for this method.

			return simulationTime;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::SetSimulationTime(UInt64 newTime)
		{
			if(disposed)
				throw ObjectDisposedException("DeadlockDetectionSimulationManager", "The DeadlockDetectionSimulationManager has been disposed");

			// Here, we must consider a lock-based or lock-free implementation for this method.
			simulationTime = newTime;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::PrepareForParallelSimulation()
		{
			if(disposed)
				throw ObjectDisposedException("DeadlockDetectionSimulationManager", "The DeadlockDetectionSimulationManager has been disposed");

			if(hasPreparedForParallelSimulation)
				return ;

			// Here, we must consider NUMA-aware thread programming styles.
			//Int32 originalNumberOfCores = Environment::get_ProcessorCount();
			//Int32 numberOfCores = originalNumberOfCores;
			UInt32 originalNumberOfCores = numOfLogicalProcessors;
			UInt32 numberOfCores = originalNumberOfCores;

			if(originalNumberOfCores <= 1)
			{
				numberOfCores = 2;
				originalNumberOfCores = 2;
			}

			if(objects->Size() < numberOfCores)
				numberOfCores = objects->Size();
			// ***************************** Do Clustering **************************** //
			printf("PS2 clustering phase ...\n");
			// Consider here to release objs.
			/*vector<LogicalProcess *> *objs = new vector<LogicalProcess *>;
			for(Int32 i = 0 ; i < objects->Size() ; i++)
			objs->push_back((*objects)[i]);
			partitions = partitioner->Partition(objs, numberOfCores);*/

			partitions = partitioner->Partition(objects, numberOfCores);

			///if(numberOfCores > originalNumberOfCores)
			///	throw ArgumentOutOfRangeException("numberOfPartitions", "The updated numberOfPartitions is greater than the requested value by the simulation kernel");

			//deadlock_counter = LockFreeCounter(numberOfCores);
			threadCounter = 0;
			global_deadlock_counter = LockFreeCounter(numberOfCores);
			/*Int32 numberOfClusterEvents = nearestPower(numberOfCores);
			clusterEvents = new EventInterface *[numberOfClusterEvents];
			for(register Int32 i = 0 ; i < numberOfClusterEvents ; i++)
			clusterEvents[i] = new EventInterface();*/
			///printf("global_deadlock_counter: %d numberOfCores: %d\n", global_deadlock_counter.get_Count(), numberOfCores);
			//global_deadlock_counter_ = numberOfCores;
			///deadlock_counter = numberOfCores;
			///mutex_threadCounter = new Mutex();
			///mutex_deadlock_counter = new Mutex();

			for(register UInt32 i = 0 ; i < numberOfCores ; i++)
				clusters.PushBack(new Cluster(i, numberOfCores, channelCapacity, this));

			if(partitions == null)
				throw ArgumentNullException("Partitioner.Partition()", "Partitioner.Partition() returned a null value");

			for(register UInt32 currentClusterID = 0 ; currentClusterID < partitions->GetNumberOfPartitions() ; currentClusterID++)
			{
				Vector<LogicalProcess *> *partition = partitions->GetPartitionByIndex(currentClusterID);

#if defined __Parvicursor_xSim_Debug_Enable__
				std::cout << "============================================================" << std::endl;
				std::cout << "In Cluster: " << currentClusterID << std::endl;
#endif

				for(register UInt32 i = 0 ; i < partition->Size() ; i++)
				{
					// Source
					LogicalProcess *src = (*partition)[i];
#if defined __Parvicursor_xSim_Debug_Enable__
					std::cout << "LP Name: " << src->GetName().get_BaseStream() << " GetOutputLpCount(): " << src->GetOutputLpCount() << std::endl;
#endif
					for(register UInt32 j = 0 ; j < src->GetOutputLpCount() ; j++)
					{
						// Destination
						LogicalProcess *dest = GetLogicalProcessById(src->GetOutputLpId(j));
						UInt32 destClusterID = dest->GetOwnerClusterID();
#if defined __Parvicursor_xSim_Debug_Enable__
						std::cout << "currentClusterID: " << currentClusterID << " destClusterID: " << destClusterID << std::endl;
#endif
						if(currentClusterID != destClusterID)
						{
							if(!clusters[currentClusterID]->ContainsDestinationCluster(destClusterID))
							{
								//std::cout << "Hello !" << " currentClusterID: " << currentClusterID << " destClusterID: " << destClusterID << std::endl;
								DeadlockDetectionPriorityQueue *destInputChannel = clusters[destClusterID]->AddInputChannel();
								clusters[currentClusterID]->AddDestionationCluster(destClusterID, destInputChannel);
							}
						}
					}
				}

#if defined __Parvicursor_xSim_Debug_Enable__
				std::cout << "GetDestinationClusterCount(): " << clusters[currentClusterID]->GetDestinationClusterCount() << std::endl;

				std::cout << "Destination channels: ";
				for(Int32 i = 0 ; i < clusters.Size() ; i++)
					std::cout << clusters[currentClusterID]->GetDestinationClusterChannelByID(i) << ", ";
				std::cout << std::endl;
#endif
			}

#if defined __Parvicursor_xSim_Debug_Enable__
			// Prints the information of clusters
			for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
			{
				std::cout << "------------------------------------------------\n";
				Vector<LogicalProcess *> *partiotion= partitions->GetPartitionByIndex(i);
				Cluster *cluster = clusters[i];
				std::cout << "Cluster Information " << i << "\n";
				std::cout << "Number of Input Channels: " << cluster->GetInputChannelCount() << "\n" ;
				std::cout << "Number of Output Channels (Destination Clusters): " << cluster->GetDestinationClusterCount() << "\n" ;
				if(cluster->GetDestinationClusterCount() > 0)
				{
					std::cout << "Destination Clusters' ID: ";
					for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
						if(cluster->GetDestinationClusterChannelByID(i) != null)
							std::cout << i << ", ";
					std::cout << "\n";
				}

				/*if(partiotion->Size() > 0)
				{
					std::cout << "Logical Processes: ";
					for(register UInt32 i = 0 ; i < partiotion->Size() ; i++)
						std::cout << (*partiotion)[i]->GetName().get_BaseStream() << "(" << (*partiotion)[i]->GetID() << "), ";
					std::cout << "\n";
				}*/
				std::cout << std::endl;
			}

			std::cout << "------------------------------------------------\n";
#endif

            GenerateDotGraph("partitioning.dot");

			if(on_partitioning_completion_callback != null)
				on_partitioning_completion_callback(on_partitioning_completion_state);

			//exit(0);
			// ***************************** Thread Creation ************************* //
			mutex_completion = new Mutex();
			waitcv_completion = new ConditionVariable(mutex_completion);

			mutex_simulation_phases = new Mutex();
			waitcv_simulation_phases = new ConditionVariable(mutex_simulation_phases);

			///mutex_controller = new Mutex();
			///waitcv_controller = new ConditionVariable(mutex_controller);
			controller = new Thread(Wrapper_To_Call_Controller, (void *)this);
			pthread_spin_init(&lock, pshared);

			//std::cout << "numberOfCores " << numberOfCores << " controller " << controller << std::endl;

			controller->Start();
			controller->SetDetached();

			for(register UInt32 i = 0 ; i < numberOfCores ; i++)
			{
				workers.PushBack(new Thread(Wrapper_To_Call_Worker, (void *)this));
				workers[i]->Start();
			}

			for(register UInt32 i = 0 ; i < numberOfCores ; i++)
				workers[i]->SetDetached();

			hasPreparedForParallelSimulation = true;

		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Worker()
		{
			//Int32 ClusterID = (Int32)threadCounter.get_Count_And_Increment();
			UInt32 ClusterID = -1;
			//mutex_threadCounter->Lock();
			pthread_spin_lock(&lock);
			ClusterID = threadCounter;
			threadCounter++;
			pthread_spin_unlock(&lock);
			//mutex_threadCounter->Unlock();
			//std::cout << "threadCounter.get_Count(): " << threadCounter.get_Count() << std::endl;
			Cluster *cluster = clusters[ClusterID];
			pri_queue_lc *localQueue = cluster->GetLocalPriorityQueue();
			Vector<LogicalProcess *> *partiotion= partitions->GetPartitionByIndex(ClusterID);
			UInt64 localSimulationTime = 0;
			//if(ClusterID == 1) Thread::Sleep(100000);

#if defined __Parvicursor_xSim_Debug_Enable__
			// ************************** Prints Cluster Information ********************** //
			mtx->Lock();
			std::cout << "------------------------------------------------\n";
			std::cout << "I'm thread working on cluster " << ClusterID << "\n";
			std::cout << "Cluster Information \n";
			std::cout << "Number of Input Channels: " << cluster->GetInputChannelCount() << "\n" ;
			std::cout << "Number of Output Channels (Destination Clusters): " << cluster->GetDestinationClusterCount() << "\n" ;
			if(cluster->GetDestinationClusterCount() > 0)
			{
				std::cout << "Destination Clusters' ID: ";
				for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
					if(cluster->GetDestinationClusterChannelByID(i) != null)
						std::cout << i << ", ";
				std::cout << "\n";
			}

			if(partiotion->Size() > 0)
			{
				std::cout << "Logical Processes: ";
				for(register UInt32 i = 0 ; i < partiotion->Size() ; i++)
				{
                    LogicalProcess *lp = (*partiotion)[i];
					std::cout << "(" << lp->GetID() << ", "<< lp->GetName().get_BaseStream() << "), ";
                }
				std::cout << "\n";
			}

			std::cout << std::endl;
			mtx->Unlock();
#endif
			// ******************* Performs Conservative Simulation ******************** //

			if(ClusterID == 0)
			{
				mtx->Lock(); std::cout << "PS2 Running ..." << std::endl; mtx->Unlock();
			}

			for(register UInt32 i = 0 ; i < partiotion->Size() ; i++)
				(*partiotion)[i]->Elaborate();

			mutex_simulation_phases->Lock();
			{
				completedThreadCount_simulation_phases++;
				if(completedThreadCount_simulation_phases == clusters.Size())
				{
					///printf("1 ---- Cluster %d\n", ClusterID);
					completedThreadCount_simulation_phases = 0;
					simulation_phases_met = true;
					waitcv_simulation_phases->Broadcast();
				}
				else
				{
					//printf("2 ---- Cluster %d\n", ClusterID);
					//while(!simulation_phases_met)
					waitcv_simulation_phases->Wait();
				}
			}
			mutex_simulation_phases->Unlock();
			// ********************************** End Barrier Phase ************************************** //

			for(register UInt32 i = 0 ; i < partiotion->Size() ; i++)
				(*partiotion)[i]->Initialize();

			// ********************************** Begin Barrier Phase ************************************** //
			mutex_simulation_phases->Lock();
			{
				completedThreadCount_simulation_phases++;
				if(completedThreadCount_simulation_phases == clusters.Size())
				{
					///printf("1 ---- Cluster %d\n", ClusterID);
					completedThreadCount_simulation_phases = 0;
					simulation_phases_met = true;
					waitcv_simulation_phases->Broadcast();
				}
				else
				{
					//printf("2 ---- Cluster %d\n", ClusterID);
					//while(!simulation_phases_met)
					waitcv_simulation_phases->Wait();
				}
			}
			mutex_simulation_phases->Unlock();

			//printf("After elaborate in Cluster %d\n", ClusterID);
			//exit(0);
			//if(ClusterID == 0)
			//    simulation_phases_met = false;
			//printf("x ---- Cluster %d\n", ClusterID);
			//Thread::Sleep(10000000);
			if(ClusterID == clusters.Size() - 1)
				xParvicursor_gettimeofday(&start, null);
			// ********************************** End Barrier Phase ************************************** //

			PdesTime minimum = -1;
			Int32 index = -2;
			PdesTime timestamp = -1;
			EventInterface *nextEvent = null;
			EventInterface *nextEventMinimum = null;
			bool globalTerminationHasRequested = false;
			DeadlockDetectionPriorityQueue *pq = null;
			DeadlockDetectionPriorityQueue *inputQueue;
			EventType eventType;
			LogicalProcess *receiver = null;
			PdesTime pri;
			bool terminationHasRequestedByUser = false;

            //cluster->messages = new Vector<std::pair<EventInterface *, Int32> >;//(1 + cluster->GetInputChannelCount());
            //cluster->messagesIndex = 0;
           // std::cout << "cluster->messages->Size() " << cluster->messages->Size() << std::endl;
			///std::cout << "localQueue: " << localQueue << std::endl;
			// We must execute the events from localQueue if we are at the beginning of the simulation
			/*if(localSimulationTime == 0)
			{
				nextEvent = (EventInterface *)priq_top(localQueue, &pri);
				if(nextEvent != null)
				{
					nextEvent = (EventInterface *)priq_pop(localQueue, &pri);
					timestamp = pri.timestamp;
					///mtx->Lock(); std::cout << "x1 Cluster " << ClusterID << " localSimulationTime " << localSimulationTime << " minimum " << timestamp << std::endl; mtx->Unlock();
					localSimulationTime = timestamp;
					cluster->SetLocaltime(localSimulationTime);
					receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
					receiver->SetSimulationTime(timestamp);
					//receiver->pq = localQueue;
					receiver->e = nextEvent;
					///nextEvent->Print(__LINE__);
					receiver->ExecuteProcess();
					if(nextEvent->deletable)
						receiver->DeallocateEvent(nextEvent);///
				}
			}*/

			/*if(localSimulationTime == 0)
			{
				nextEvent = (EventInterface *)priq_top_lc(localQueue, &pri);
				if(nextEvent != null)
				{
					//nextEvent = (EventInterface *)priq_pop(localQueue, &pri);
					timestamp = pri;
					//mtx->Lock(); std::cout << "x1 Cluster " << ClusterID << " localSimulationTime " << localSimulationTime << " minimum " << timestamp << std::endl; mtx->Unlock();
					localSimulationTime = timestamp._time;
					cluster->SetLocaltime(localSimulationTime);
					//receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
					//receiver->SetSimulationTime(timestamp);
                    std::pair<EventInterface *, Int32> p;
                    p.first = nextEvent;
                    p.second = -1;
                    cluster->messages->PushBack(p);
					//nextEvent->Print(__LINE__);
					//receiver->ExecuteProcess();
				}

				//exit(0);

                //Int32 i = 0;

                cluster->messagesIndex = 0;
                for(cluster->messagesIndex = 0 ; cluster->messagesIndex < cluster->messages->Size() ; )
                {
                    std::pair<EventInterface *, Int32> p = (*cluster->messages)[cluster->messagesIndex];
                    nextEvent = p.first;
                    timestamp = nextEvent->GetReceiveTimeLc();
                    eventType = nextEvent->eventType;
                    //std::cout << "i " << i << std::endl;

                    if(eventType == NormalMessage)
                    {
                        receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
                        receiver->SetSimulationTime(localSimulationTime);
                        receiver->ExecuteProcess();
                    }
                    else if(eventType == TerminationHasRequestedMessageByUser)
                    {
                        terminationHasRequestedByUser = true;
                        delete nextEvent; // delete messges in ~cluster;
                        goto EndWhile;
                    }
                }
			}*/

			//std::cout << "channelCount " << cluster->GetInputChannelCount() << std::endl;

			//mtx->Lock(); std::cout << "cluster->messagesIndex " << cluster->messagesIndex << " cluster->messages->Size() " << cluster->messages->Size() << std::endl; mtx->Unlock();

			//EventInterface **inputEvents = new EventInterface*[1 + cluster->GetInputChannelCount()];
			//RingBuffer delta_q = RingBuffer(cluster->GetInputChannelCount());
BeginWhile:
			//while(localSimulationTime <= simulateUntil)
			while(localSimulationTime < simulateUntil)
			{
				index = -2;
				//minimum = -1;
                //cluster->messages->Reset();\
                cluster->messagesIndex = 0;///

				nextEvent = (EventInterface *)priq_top_lc(localQueue, &minimum);
				if(nextEvent != null)
				{
					//timestamp = pri;
					//minimum = pri;//timestamp;
					//minimum = pri;
					nextEventMinimum = nextEvent;
					index = -1;

                    //std::pair<EventInterface *, Int32> p;\
                    p.first = nextEvent;\
                    p.second = -1;\
                    cluster->messages->PushBack(p);
				}

				// If the cluster has not any input channel and there is not any event in the local queue,
				// then the execution of this cluster is to be terminated. We must inform the controller thread
				// of this happening (e.g, there may be a global termination point after this time).
				else if(cluster->workingInputChannelCount == 0)
					goto EndWhile;

				//Int32 initIndex = cluster->GetInputChannelCount() - 1;
				//for(register Int32 i = initIndex ; i != -1  ; i--)
				//Int32 initIndex = cluster->GetInputChannelCount() - 1;
				//for(i = initIndex ; i != -1  ; --i)
				for(register UInt32 i = 0  ; i < cluster->GetInputChannelCount() ; i++)
				{
					//for(register Int32 i = 0  ; i < cluster->GetInputChannelCount() ; i++)
					//std::cout << "Cluster ID: " << ClusterID << " Peek for Input Channel: " << i << std::endl;

					inputQueue = cluster->GetInputChannelByIndex(i);

					nextEvent = (EventInterface *)inputQueue->Peek();
					timestamp = nextEvent->GetReceiveTimeLc();
					/*if(nextEvent == null)
					{
						printf("nextEvent == null 2\n");
						//continue;
						exit(0);
					}*/

					//timestamp = nextEvent->GetReceiveTime();

					eventType = nextEvent->GetEventType();

					// Handles the termination detection during the simulation reported by the control thread.
					if(eventType == TerminationHasRequestedMessage)
					{
						nextEvent = inputQueue->Dequeue();
                        /// Logical clock settings
                        /// The algorithm for receiving a message:
                        /// (message, time_stamp) = receive();
                        /// time = max(time_stamp, time) + 1;
                        //cluster->lc_time = max(cluster->lc_time, nextEvent->lc) + 1;
						globalTerminationHasRequested = true;
						delete nextEvent;
						printf("TerminationHasRequestedMessage1 in Cluster %d\n", ClusterID);
						goto BeginWhile;
					}

					// Handles the deadlock recovery during the deadlocked simulation reported by the control thread.
					else if(eventType == RecoveryHasRequestedMessage)
					{
                        /// Logical clock settings
                        /// The algorithm for receiving a message:
                        /// (message, time_stamp) = receive();
                        /// time = max(time_stamp, time) + 1;
                        //cluster->lc_time = max(cluster->lc_time, nextEvent->lc) + 1;
						nextEvent = inputQueue->Dequeue();
						//mtx->Lock(); std::cout << "x0 Cluster " << ClusterID << " localSimulationTime " << localSimulationTime << " minimum " << timestamp <<std::endl; mtx->Unlock();
						delete nextEvent;
						nextEvent = null;
						///mtx->Lock(); std::cout << "x2 Cluster " << ClusterID << " localSimulationTime " << localSimulationTime << " minimum " << timestamp << " sender: " << nextEvent->GetSenderReference()->GetName().get_BaseStream() << " receiver: " << nextEvent->GetReceiverReference()->GetName().get_BaseStream() <<std::endl; mtx->Unlock();
						//mtx->Lock();printf("RecoveryHasRequestedMessage1 received in Cluster %d in timestamp: %d deadlock_counter: %d\n", ClusterID, (int)timestamp, (int)global_deadlock_counter.get_Count());  fflush(stdout); mtx->Unlock();
						// ********************************** Begin Barrier Phase ************************************** //
						///mtx->Lock();printf("Before in Cluster %d\n", ClusterID);mtx->Unlock();
						DeadlockRecoveryBarrierPhase(/*"Cluster " + to_string(ClusterID)*/);
						///mtx->Lock();printf("After in Cluster %d\n", ClusterID);mtx->Unlock();
						// ********************************** End Barrier Phase **************************************

						pq = cluster->GetMinimumTimestampQueue();
						index = -1;
						/*if(pq == null)
						{
							if(cluster->GetMinimumTimestamp() != -1)
							{
								nextEvent = (EventInterface *)priq_pop(localQueue, &pri);
							}
							//std::cout << "pq is null cluster->GetMinimumTimestamp() " << cluster->GetMinimumTimestamp() << std::endl;
							else
								goto EndWhile;
						}
						else
							nextEvent = pq->Dequeue();*/

                        if(pq == null)
                        {
                            if(cluster->GetMinimumTimestamp() != -1)
							{
								nextEvent = (EventInterface *)priq_top_lc(localQueue, &pri);
								//nextEvent = (EventInterface *)priq_pop_lc(localQueue, &pri);
								index = -1;
							}
							//std::cout << "pq is null cluster->GetMinimumTimestamp() " << cluster->GetMinimumTimestamp() << std::endl;
							else
								goto EndWhile;
                        }
                        else
                        {
                            nextEvent = pq->Peek();
                            //nextEvent = pq->Dequeue();
                            index = pq->ownerInputIndex;
                        }

						EventType eventType = nextEvent->GetEventType();

						if(eventType == TerminationHasRequestedMessage)
						{
							globalTerminationHasRequested = true;
							//delete nextEvent;
							printf("From recoveryHasRequested TerminationHasRequestedMessage2 in Cluster %d\n", ClusterID);
							goto EndWhile;
						}
                        else if(eventType == TerminationHasRequestedMessageByUser)
                        {
                            terminationHasRequestedByUser = true;
                            //delete nextEvent;
                            goto EndWhile;
                        }
                        else if(eventType == RequestToRemoveTheChannelMessage) /////////// For sc_stop
                        {
                            cluster->GetInputChannelByIndex(index)->Dequeue();
                            cluster->RemoveInputChannelByIndex(index);
                            mtx->Lock(); printf("Receiving RequestToRemoveTheChannelMessage1 in Cluster %d from Cluster %d timestamp %d GetLocaltime %d workingInputChannelCount %d index %d\n", ClusterID, nextEvent->GetSenderReference(), (Int32)timestamp._time, (Int32)cluster->GetLocaltime(), cluster->workingInputChannelCount, index); mtx->Unlock();
                            delete nextEvent;
                            goto EndWhile;
                            //initIndex = cluster->GetInputChannelCount() - 1;
                            //std::cout << "RequestToRemoveTheChannelMessage " << std::endl;
                            //exit(0);
                        }

                        localSimulationTime = timestamp._time;

						if(localSimulationTime >= simulateUntil)
						{
                            //if(nextEvent->deletable)
                                //receiver->DeallocateEvent(nextEvent);
                            //std::cout << "hhhhh" << std::endl;
                            //delete nextEvent;
                            goto EndWhile;
						}

						//std::cout << "deadlock index " << index << std::endl;

                        //std::pair<EventInterface *, Int32> p;\
                        p.first = nextEvent;\
                        p.second = index;\
                        cluster->messagesIndex = 0;\
                        cluster->messages->Reset();\
                        cluster->messages->PushBack(p);

						localSimulationTime = timestamp._time;
						cluster->SetLocaltime(localSimulationTime);
						receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
						receiver->SetSimulationTime(localSimulationTime);
						receiver->pq = pq;
						receiver->e = nextEvent;
						receiver->index = index;
						//nextEvent->Print(__LINE__);
                        //std::cout << "type " << nextEvent->GetEventType() << " index " << index << " messagesInsex " << cluster->messagesIndex << std::endl;
                        receiver->ExecuteProcess();
                        //if(nextEvent->deletable)
                        //    receiver->DeallocateEvent(nextEvent);///

						//mtx->Lock(); std::cout << "2 Cluster " << ClusterID << " count " << cluster->deltaQueue->GetCount() << std::endl; mtx->Unlock();
						goto BeginWhile;
					}

					//std::cout << "Cluster ID: " << ClusterID << " Peeked Input Channel from Cluster: " << nextEvent->GetSenderReference()->GetOwnerClusterID() << " for Input Channel: " << i << std::endl;

					if(index == -2 && i == 0)
					//if(i == initIndex && index == -2)
					{
						minimum = timestamp;
						nextEventMinimum = nextEvent;
						index = i;

                        //std::pair<EventInterface *, Int32> p;\
                        p.first = nextEvent;\
                        p.second = i;\
                        cluster->messages->Reset();\
                        cluster->messages->PushBack(p);
					}
					else
					{
						if(timestamp < minimum)
						{
							minimum = timestamp;
							nextEventMinimum = nextEvent;
							index = i;

                            //std::pair<EventInterface *, Int32> p;\
                            p.first = nextEvent;\
                            p.second = i;\
                            cluster->messages->Reset();\
                            cluster->messages->PushBack(p);
						}
						//else if(timestamp._time == minimum._time)\
						{\
                            std::pair<EventInterface *, Int32> p;\
                            p.first = nextEvent;\
                            p.second = i;\
                            cluster->messages->PushBack(p);\
						}
					}
				}

                localSimulationTime = minimum._time;
                cluster->SetLocaltime(localSimulationTime);
                //cluster->messagesIndex = 0;
                /// Logical clock settings
                /// The algorithm for receiving a message:
                /// (message, time_stamp) = receive();
                /// time = max(time_stamp, time) + 1;
                //cluster->lc_time = max(cluster->lc_time, nextEventMinimum->lc) + 1;
				//for(cluster->messagesIndex = 0 ; cluster->messagesIndex < cluster->messages->Size() ; )
                {
                    //std::pair<EventInterface *, Int32> p = (*cluster->messages)[cluster->messagesIndex];\
                    nextEvent = p.first;
                    //if(index == -1)
                    //    priq_pop_lc(localQueue, &pri);
                    //else
                    //    cluster->GetInputChannelByIndex(index)->Dequeue();

                    nextEvent = nextEventMinimum;
                    eventType = nextEvent->eventType;

                    if(eventType == NormalMessage/* || eventType == AppNullMessage*/)
                    {
                        /*if(index == -1)
                        {
                            priq_pop_lc(cluster->localQueue, &pri);
                        }
                        else
                            cluster->GetInputChannelByIndex(index)->Dequeue();*/

                        receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
                        receiver->e = nextEvent;//
                        receiver->index = index;
                        receiver->SetSimulationTime(localSimulationTime);
                        receiver->ExecuteProcess();
                        /*if(nextEvent->deletable)
                            receiver->DeallocateEvent(nextEvent);*/
                    }
                    else if(eventType == TerminationHasRequestedMessageByUser)
                    {
                        /// Logical clock settings
                        /// The algorithm for receiving a message:
                        /// (message, time_stamp) = receive();
                        /// time = max(time_stamp, time) + 1;
                        //cluster->lc_time = max(cluster->lc_time, nextEvent->lc) + 1;
                        terminationHasRequestedByUser = true;
                        delete nextEvent;
                        goto EndWhile;
                    }
                    else if(eventType == RequestToRemoveTheChannelMessage)
                    {
                        //std::cout << "ClusterID " << ClusterID << " p.second " << p.second << std::endl;
                        //cluster->GetInputChannelByIndex(i)->Dequeue();
                        //cluster->RemoveInputChannelByIndex(p.second);
                        cluster->GetInputChannelByIndex(index)->Dequeue();
                        cluster->RemoveInputChannelByIndex(index);
                        //initIndex = cluster->GetInputChannelCount() - 1;
                        //cluster->messagesIndex++;
                        /// Logical clock settings
                        /// The algorithm for receiving a message:
                        /// (message, time_stamp) = receive();
                        /// time = max(time_stamp, time) + 1;
                        //cluster->lc_time = max(cluster->lc_time, nextEvent->lc) + 1;
                        //delete nextEvent;
                        goto BeginWhile;
                        //if(cluster->workingInputChannelCount == 0)
                        //goto EndWhile;
                    }

                    //receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
                    //receiver->SetSimulationTime(localSimulationTime);
                    //receiver->e = nextEvent;
                    //receiver->ExecuteProcess();
                    //if(nextEvent->deletable)
                    //receiver->DeallocateEvent(nextEvent); delete them in an outer loop.
                }

                //if(cluster->messages->Size() > 1)
                //    std::cout << "cluster->messages->Size() " << cluster->messages->Size() << std::endl;
                /*for(cluster->messagesIndex = 0 ; cluster->messagesIndex < cluster->messages->Size() ; cluster->messagesIndex++)
                {
                    std::pair<EventInterface *, Int32> p = (*cluster->messages)[cluster->messagesIndex];
                    nextEvent = p.first;
                    eventType = nextEvent->eventType;

                    if(eventType == NormalMessage || eventType == AppNullMessage)
                    {
                        receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
                        if(nextEvent->deletable)
                            receiver->DeallocateEvent(nextEvent);
                    }
                    else if(eventType == RequestToRemoveTheChannelMessage)
                    {
                        delete nextEvent;
                    }
                }*/

                // Checks the local priority queue
                /*cluster->messages->Reset();
                nextEvent = (EventInterface *)priq_top(localQueue, &pri);
                if(nextEvent != null && nextEvent->GetReceiveTime() == minimum)
                {
                    std::pair<EventInterface *, Int32> p;
                    p.first = nextEvent;
                    p.second = -1;
                    cluster->messages->PushBack(p);
                    goto LoopAgain;
                }*/
                //cluster->messages->Reset();
				/*if(index == -1)
					nextEventMinimum = (EventInterface *)priq_pop(localQueue, &pri);
				else if(index == -2)
					goto EndWhile;
				else
				{
					pq = cluster->GetInputChannelByIndex(index);
					nextEventMinimum = pq->Dequeue();
					//pq->lookahead = nextEventMinimum->GetReceiveTime() - nextEventMinimum->GetSendTime();////
				}

				localSimulationTime = minimum;
				cluster->SetLocaltime(localSimulationTime);
				//mtx->Lock(); std::cout << "Cluster " << ClusterID << " localSimulationTime " << localSimulationTime << " nextEventMinimum->GetDeltaCounter() " << nextEventMinimum->GetDeltaCounter() << " count " << cluster->deltaQueue->GetCount() << std::endl; mtx->Unlock();

				eventType = nextEventMinimum->eventType;

				if(eventType == NormalMessage)
				{
                    ///nextEventMinimum->Print(__LINE__);
					//cluster->nonLocalSentMessages = 0;
					receiver = LogicalProcess::GetLogicalProcessById(nextEventMinimum->GetReceiverReference());
					receiver->SetSimulationTime(localSimulationTime);
					//receiver->pq = pq;
					receiver->e = nextEventMinimum;
					receiver->ExecuteProcess();

					// Send null messages here based the traditional CMB protocol.
					//if(LogicalProcess::GetLogicalProcessById(nextEventMinimum->GetSenderReference())->GetOwnerClusterID() != receiver->GetOwnerClusterID())
						//if(cluster->nonLocalSentMessages == 0)
						//	SendNullMassage(ClusterID); // one-output gate ?

					if(nextEventMinimum->deletable)
						receiver->DeallocateEvent(nextEventMinimum);///
				}
                else if(eventType == TerminationHasRequestedMessageByUser)
				{
                    terminationHasRequestedByUser = true;
					delete nextEventMinimum;
					goto EndWhile;
				}
				//else if(eventType == NullMessage)\
				{ \
					//std::cout << "Receiving NullMessage in Cluster" << ClusterID << std::endl;\
					//nextEventMinimum = (EventInterface *)pq->Dequeue(timestamp);\
					delete nextEventMinimum;\
					// Send null messages here based the traditional CMB protocol.\
					//HandleNullMassage(nextEventMinimum, ClusterID);\
				}
				else if(eventType == RequestToRemoveTheChannelMessage)
				{
					//nextEvent = (EventInterface *)pq->Dequeue();
					cluster->RemoveInputChannelByIndex(index);
					initIndex = cluster->GetInputChannelCount() - 1;
					///mtx->Lock(); printf("Receiving RequestToRemoveTheChannelMessage1 in Cluster %d from Cluster %d timestamp %d GetLocaltime %d workingInputChannelCount %d index %d\n", ClusterID, nextEvent->GetSenderReference(), (Int32)timestamp, (Int32)cluster->GetLocaltime(), cluster->workingInputChannelCount, index); mtx->Unlock();
					delete nextEventMinimum;
					//continue;
				}*/
			}

EndWhile:

			//printf("---------EndWhile-------- in Cluster %d localSimulationTime: %d localQueueCount: %d\n", ClusterID, (Int32)localSimulationTime, localQueue->length); ///
			/*for(int i = 0 ; i < clusters.Size(); i++)
			printf("Hello in Cluster %d localSimulationTime %d\n", i, clusters[i]->GetLocaltime()); ///*/
            if(terminationHasRequestedByUser)
            {
                //for(i = cluster->_destinationClusters.Size() - 1 ; i != -1 ; i--)
                for(register Int32 i = 0 ; i < cluster->_destinationClusters.Size() ; i++)
                {
                    DeadlockDetectionPriorityQueue *outputChannel = cluster->_destinationClusters[i];
                    //printf("ClusterID: %d DestinatonCluster: %d Count: %d\n", ClusterID, i, outputChannel->GetCount());
                    EventInterface *newEvent = new EventInterface();
                    UInt64 time = cluster->GetLocaltime();//cluster->GetLatestSendTime() + 1;
                    newEvent->SetSenderReference(ClusterID);
                    newEvent->SetEventType(TerminationHasRequestedMessageByUser);
                    newEvent->SetReceiveTime(time);
                    newEvent->SetSendTime(time);
                    outputChannel->Enqueue(newEvent);
                    //mtx->Lock(); printf("Sending RequestToRemoveTheChannelMessage from Cluster %d to Cluster %d time %d GetLocaltime %d\n", ClusterID, i, (Int32)time, (Int32)cluster->GetLocaltime()); mtx->Unlock();
                }
            }
            else
            {
                //for(i = cluster->_destinationClusters.Size() - 1 ; i != -1 ; i--)
                for(register Int32 i = 0 ; i < cluster->_destinationClusters.Size() ; i++)
                {
                    DeadlockDetectionPriorityQueue *outputChannel = cluster->_destinationClusters[i];
                    //printf("ClusterID: %d DestinatonCluster: %d Count: %d\n", ClusterID, i, outputChannel->GetCount());
                    EventInterface *newEvent = new EventInterface();
                    UInt64 time = cluster->GetLocaltime();//cluster->GetLatestSendTime() + 1;
                    newEvent->SetSenderReference(ClusterID);
                    newEvent->SetEventType(RequestToRemoveTheChannelMessage);
                    newEvent->SetReceiveTime(time);
                    newEvent->SetSendTime(time);
                    outputChannel->Enqueue(newEvent);
                    //mtx->Lock(); printf("Sending RequestToRemoveTheChannelMessage from Cluster %d to Cluster %d time %d GetLocaltime %d\n", ClusterID, i, (Int32)time, (Int32)cluster->GetLocaltime()); mtx->Unlock();
                }
			}

			cluster->SetTerminated(true);
			Long count_ = global_deadlock_counter.Decrement_And_get_Count();
			if(count_ == 0)
			{
				//std::cout << "hellllllllllllllllllllllllllllllo" << std::endl;
				global_mutex_controller->Lock();
				global_met = true;
				global_waitcv_controller->Signal();
				global_mutex_controller->Unlock();
			}

			// ********************************** Begin Barrier Phase ************************************** //
			mutex_simulation_phases->Lock();
			{
				completedThreadCount_simulation_phases++;
				if(completedThreadCount_simulation_phases == clusters.Size())
				{
					simulation_phases_met = true;
					completedThreadCount_simulation_phases = 0;
					waitcv_simulation_phases->Broadcast();
				}
				else
					//while(!simulation_phases_met)
					waitcv_simulation_phases->Wait();
				//simulation_phases_met = false;
			}
			mutex_simulation_phases->Unlock();
			// ********************************** End Barrier Phase ************************************** //

			global_mutex_controller->Lock();
			controllerThreadTerminationRequested = true;
			global_waitcv_controller->Signal();
			global_mutex_controller->Unlock();

			if(ClusterID == clusters.Size() - 1)
				xParvicursor_gettimeofday(&stop, null);

			for(register Int32 i = 0 ; i < partiotion->Size() ; i++)
				(*partiotion)[i]->Finalize();

			mutex_completion->Lock();
			completedThreadCount++;
			waitcv_completion->Signal();
			mutex_completion->Unlock();
		}
		//----------------------------------------------------
        EventInterface *DeadlockDetectionSimulationManager::PeekNextEvent(LogicalProcess *lp)
        {
            //return null;
            /*Cluster *cluster = clusters[lp->GetOwnerClusterID()];

            PdesTime pri;
            EventInterface *nextEvent = (EventInterface *)priq_top_lc(cluster->localQueue, &pri);
            if(nextEvent != null && pri._time == cluster->GetLocaltime())
            {
                lp->e = nextEvent;
                lp->index = -1;
                return nextEvent;
            }
            else
                return null;*/

            Cluster *cluster = clusters[lp->GetOwnerClusterID()];

			PdesTime minimum, timestamp;
			Int32 index = -2;
			EventInterface *nextEvent;
			EventInterface *nextEventMinimum = null;
			DeadlockDetectionPriorityQueue *inputQueue;
			EventType eventType;

            nextEvent = (EventInterface *)priq_top_lc(cluster->localQueue, &minimum);
            if(nextEvent != null)
            {
                nextEventMinimum = nextEvent;
                index = -1;
            }

            for(register UInt32 i = 0  ; i < cluster->GetInputChannelCount() ; i++)
            //UInt32 count = cluster->GetInputChannelCount();
            //for(register UInt32 i = count  ; i != 0 ; i--)
            {
                //inputQueue = cluster->GetInputChannelByIndex(count - i);
                inputQueue = cluster->GetInputChannelByIndex(i);

                if(inputQueue->GetCount() == 0)
                    return null;

                //std::cout << "inputQueue->GetCount() " << inputQueue->GetCount() << std::endl;

                nextEvent = (EventInterface *)inputQueue->Peek();
                timestamp = nextEvent->GetReceiveTimeLc();
                eventType = nextEvent->GetEventType();

                if(eventType != NormalMessage)
                    return null;

                if(index == -2 && i == 0)
                {
                    minimum = timestamp;
                    nextEventMinimum = nextEvent;
                    index = i;
                }
                else
                {
                    if(timestamp < minimum)
                    {
                        minimum = timestamp;
                        nextEventMinimum = nextEvent;
                        index = i;
                    }
                }
            }

            if(nextEventMinimum != null)
            {
                if(nextEventMinimum->GetReceiveTime() == lp->GetSimulationTime())
                {
                    if(nextEventMinimum->GetReceiverReference() == lp->GetID())
                    {
                        lp->e = nextEventMinimum;
                        lp->index = index;
                        return nextEventMinimum;
                    }
                    else
                        return null;
                }
                else
                    return null;
            }
            else
                return null;
        }
        //----------------------------------------------------
        void DeadlockDetectionSimulationManager::SendEvent(EventInterface *e)
        {
            UInt32 senderID = e->GetSenderReference();
            LogicalProcess *sender = LogicalProcess::GetLogicalProcessById(senderID);
            UInt32 srcClusterID = sender->GetOwnerClusterID();

            Cluster *srcCluster = clusters[srcClusterID];

            /// https://en.wikipedia.org/wiki/Lamport_timestamps
            /// Logical clock settings
            /// In a Pseudocode format, the algorithm for sending:
            /// time = time + 1;
            /// time_stamp = time;
            /// send(message, time_stamp);
            srcCluster->lc_time = srcCluster->lc_time + 1;
            e->lc = srcCluster->lc_time;

            UInt32 destClusterID = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference())->GetOwnerClusterID();

            if(srcClusterID == destClusterID)
                priq_push_lc(srcCluster->GetLocalPriorityQueue(), e, e->GetReceiveTimeLc());
            else
                srcCluster->GetDestinationClusterChannelByID(destClusterID)->Enqueue(e);
        }
        //----------------------------------------------------
        EventInterface *DeadlockDetectionSimulationManager::GetNextEvent(LogicalProcess *lp)
        {
            Cluster *cluster = clusters[lp->GetOwnerClusterID()];

            if(cluster->previous_event != null)
            {
                if(cluster->previous_event->deletable)
                {
                    LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(cluster->previous_event->GetReceiverReference());
                    receiver->DeallocateEvent(cluster->previous_event);
                }
            }

            if(lp->index == -1)
            {
                PdesTime pri;
                priq_pop_lc(cluster->localQueue, &pri);
            }
            else
            {
                cluster->GetInputChannelByIndex(lp->index)->Dequeue();
            }

            cluster->previous_event = lp->e;

            cluster->numberOfProcessedEvents++;

            /// Logical clock settings
            /// The algorithm for receiving a message:
            /// (message, time_stamp) = receive();
            /// time = max(time_stamp, time) + 1;
            cluster->lc_time = max(cluster->lc_time, lp->e->lc) + 1;
            return lp->e;
        }
		//----------------------------------------------------
		void *DeadlockDetectionSimulationManager::Wrapper_To_Call_Worker(void *pt2Object)
		{
			// explicitly cast to a pointer to DeadlockDetectionSimulationManager
			DeadlockDetectionSimulationManager *mySelf = (DeadlockDetectionSimulationManager *)pt2Object;

			// call member
			mySelf->Worker();

			return pt2Object;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::Controller()
		{
#if defined __Parvicursor_xSim_Debug_Enable__
			std::cout << "I'm controller thread" << std::endl;
#endif

			//global_mutex_controller->Lock();

			while(true) // Implement a termination mechanism for controller.
			{
				global_mutex_controller->Lock();
				{
					while(!global_met && !controllerThreadTerminationRequested)
						global_waitcv_controller->Wait();

					if(controllerThreadTerminationRequested)
					{
						global_mutex_controller->Unlock();
						goto EndWhile;
					}

					global_met = false;
				}
				global_mutex_controller->Unlock();

#if defined __Parvicursor_xSim_Debug_Enable__
				AlarmShape(4); ///
				// ****************************** Deadlock Detected Phase **************************** //
				std::cout << "\t\t\t\tI'm controller thread.\n";
				std::cout << "\tA global deadlock was detected in the conservative simulation kernel.\n";
				std::cout << "\t\t\t\tI'm going to recover from the deadlock.\n\n\n" << std::endl;
				// ****************************** Deadlock Recovery Phase **************************** //
				std::cout << "DeadlockDetectionSimulationManager::FindMinimumTimestampPerCluster()" << std::endl;
#endif
				//std::cout << "\tA global deadlock was detected in the conservative simulation kernel.\n";///

				// Finds the global minimum timestamp.
				PdesTime minimum = -1, global_minimum = -1;
				Cluster *minimumTimestampCluster = null;
				///Int32 initIndex = clusters.Size() - 1;
				// Counts the number of clusters that contain the global minimum timestamp plus the controller thread.
				global_minimum_cluster_count = 1;
				for(register Int32 clusterIndex = 0 ; clusterIndex < clusters.Size() ; clusterIndex++)
				///for(register Int32 clusterIndex = initIndex ; clusterIndex != -1 ; clusterIndex--)
				{
					if(clusters[clusterIndex]->GetTerminated())
						continue;
					minimum = FindMinimumTimestampPerCluster(clusterIndex);
					if(minimum._time == -1)
						continue;
#if defined __Parvicursor_xSim_Debug_Enable__
					std::cout << "Minimum timestamp for Cluster " << clusterIndex << ": " << minimum << std::endl;
#endif
					//std::cout << "Minimum timestamp for Cluster " << clusterIndex << ": " << minimum << std::endl;//
					if(clusterIndex == 0)
					///if(clusterIndex == initIndex)
					{
						global_minimum_cluster_count = 2;
						minimumTimestampCluster = clusters[clusterIndex];
						global_minimum = minimum;
						continue;
					}
					if(minimum._time >= 0 && global_minimum._time == -1)
					{
						global_minimum_cluster_count = 2;
						global_minimum = minimum;
						minimumTimestampCluster = clusters[clusterIndex];
						continue;
					}
					else if(minimum < global_minimum)
					{
						global_minimum = minimum;
						minimumTimestampCluster = clusters[clusterIndex];
						global_minimum_cluster_count = 2;
						continue;
					}
					else if(minimum == global_minimum)
                    {
						global_minimum_cluster_count++;
						continue;
					}
				}
				//printf("Controller::global_minimum: %d \n", (Int32)global_minimum);
#if defined __Parvicursor_xSim_Debug_Enable__
				std::cout << "Global minimum timestamp is " << global_minimum << std::endl;
#endif
				//std::cout << "Global minimum timestamp is " << global_minimum << std::endl;///

				// Now, we recovers all deadlocked clusters by allowing them to process events in their
				// input channels (queues) with respect to the global minimum timestamp.
				if(global_minimum._time >= 0)
				{
					numberOfDeadlocks++;
#if defined __Parvicursor_xSim_Debug_Enable__
					std::cout << "The deadlock recovery phase is starting ..." << std::endl;
#endif
					// Counts the number of clusters that contain the global minimum timestamp plus the controller thread.
					/*global_minimum_cluster_count = 1;
					//Int32 count1 = 0, count2 = 0, count3 = 0;
					//string g = "", bg = "", b = "";
					///for(register Int32 clusterIndex = 0 ; clusterIndex < clusters.Size() ; clusterIndex++)
					for(register Int32 clusterIndex = initIndex ; clusterIndex != -1 ; clusterIndex--)
					{
						Cluster *cluster = clusters[clusterIndex];
						if(cluster->GetTerminated())
							continue;
						//if(cluster->GetMinimumTimestamp() == global_minimum) // In a correct behavior, this must always work.
						//{
						//	count1++;
						//	g += to_string(clusterIndex) + ", ";
						//}
						if(cluster->GetBlocked() && cluster->GetMinimumTimestamp() == global_minimum )
						{
							global_minimum_cluster_count++;
							//bg += to_string(clusterIndex) + ", ";
							//count2++;
						}
						//if(cluster->GetBlocked())
						//{
						//    b += to_string(clusterIndex) + ", ";
						//	count3++;
						//}
					}*/

					//if(global_minimum == 10 && global_minimum_cluster_count == 4)
					//	Thread::Sleep(10000000);
					/*mtx->Lock();
					printf("count1(g): %d count2(b,g): %d count3(b): %d global_minimum: %d global_minimum_cluster_count: %d deadlock_counter: %d\n", count1, count2, count3, (int)global_minimum, global_minimum_cluster_count, global_deadlock_counter.get_Count());
					printf("g: %s bg: %s b: %s\n", g.c_str(), bg.c_str(), b.c_str());
					fflush(stdout);
					mtx->Unlock();*/

					//std::cout << "global_minimum_cluster_count " << global_minimum_cluster_count << std::endl;

					///for(register Int32 clusterIndex = initIndex ; clusterIndex != -1 ; clusterIndex--)
                    for(register Int32 clusterIndex = 0 ; clusterIndex < clusters.Size() ; clusterIndex++)
					{
						Cluster *cluster = clusters[clusterIndex];
						if(cluster->GetTerminated())
							continue;
						if(cluster->GetMinimumTimestamp() != global_minimum)
							continue;
						if(cluster->GetBlocked())
						{
							EventInterface *newEvent = new EventInterface();
							//EventInterface *newEvent = clusterEvents[clusterIndex];
							UInt64 time = global_minimum._time;
							newEvent->SetEventType(RecoveryHasRequestedMessage);
							newEvent->SetReceiveTime(time);
							newEvent->SetSendTime(time);
                            /// https://en.wikipedia.org/wiki/Lamport_timestamps
                            /// Logical clock settings
                            /// In a Pseudocode format, the algorithm for sending:
                            /// time = time + 1;
                            /// time_stamp = time;
                            /// send(message, time_stamp);
                            //cluster->lc_time = cluster->lc_time + 1;
                            //newEvent->lc = cluster->lc_time;
                            //newEvent->lc_cluster = cluster->GetClusterID();
							DeadlockDetectionPriorityQueue *ppq = cluster->GetBlockedPriorityQueue();
							//mtx->Lock(); printf("Sending RecoveryHasRequestedMessage from Control Thread to Cluster %d in global minimum time %d QueueCount: \n", clusterIndex, time/*ppq->GetCount()*/);  fflush(stdout); mtx->Unlock();
							cluster->GetBlockedPriorityQueue()->Enqueue(newEvent);
							//break;//
						}
						//cluster->SetBlocked(false);
					}
					DeadlockRecoveryBarrierPhase(/*"Controller"*/);
					//global_mutex_controller->Unlock();
				}
				else
				{
//#if defined __Parvicursor_xSim_Debug_Enable__
					// A negative value of the global minimum timestamp indicates a global simulation termination detection by the controller.
					std::cout << "A termination was detected in the conservative simulation kernel." << std::endl;
//#endif
					///for(register Int32 clusterIndex = initIndex ; clusterIndex != -1 ; clusterIndex--)
					for(register Int32 clusterIndex = 0 ; clusterIndex < clusters.Size() ; clusterIndex++)
					{
						Cluster *cluster = clusters[clusterIndex];
						if(cluster->GetTerminated())
							continue;
						cluster->terminationHasRequested = true;
						///for(register UInt32 i = cluster->GetInputChannelCount() - 1 ; i != -1 ; i--)
                        for(register Int32 i = 0 ; i < cluster->GetInputChannelCount() ; i++)
						{
							DeadlockDetectionPriorityQueue *inputQueue = cluster->GetInputChannelByIndex(i);
							if(inputQueue == null)
								continue;
							EventInterface *newEvent = new EventInterface();
							UInt64 time = cluster->GetLocaltime();
							newEvent->SetEventType(TerminationHasRequestedMessage);
							newEvent->SetReceiveTime(time);
							newEvent->SetSendTime(time);
                            /// https://en.wikipedia.org/wiki/Lamport_timestamps
                            /// Logical clock settings
                            /// In a Pseudocode format, the algorithm for sending:
                            /// time = time + 1;
                            /// time_stamp = time;
                            /// send(message, time_stamp);
                            //cluster->lc_time = cluster->lc_time + 1;
                            //newEvent->lc = cluster->lc_time;
                            //newEvent->lc_cluster = cluster->GetClusterID();
							inputQueue->Enqueue(newEvent);
							//printf("Sending TerminationHasRequestedMessage from Control Thread to Cluster %d\n", cluster->GetClusterID());
						}
					}
					goto EndWhile;
				}
				//AlarmShape(8); ///
			}

EndWhile:
			//global_mutex_controller->Unlock();

			return ;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::DeadlockRecoveryBarrierPhase(/*string who*/)
		{
            //std:cout << "global_minimum_cluster_count " << global_minimum_cluster_count << std::endl;
			//return;
			//if(global_minimum_cluster_count > 1)
			if(global_minimum_cluster_count > 2)
			{
                //std:cout << "global_minimum_cluster_count " << global_minimum_cluster_count << std::endl;
				deadlocked_clusters_mutex->Lock();
				{
					completedThreadCount_deadlocked_clusters_phase++;
					if(completedThreadCount_deadlocked_clusters_phase == global_minimum_cluster_count)
					{
						///printf("%s. completedThreadCount_deadlocked_clusters_phase: %d global_deadlock_counter: %d\n", who.c_str(), completedThreadCount_deadlocked_clusters_phase, global_deadlock_counter.get_Count());
						//deadlocked_clusters_met = true;
						completedThreadCount_deadlocked_clusters_phase = 0;
						//exit(0);
						deadlocked_clusters_cv->Broadcast();
					}
					else
						//while(!deadlocked_clusters_met)
						deadlocked_clusters_cv->Wait();
					//deadlocked_clusters_met = false;
				}
				deadlocked_clusters_mutex->Unlock();
			}
		}
		//----------------------------------------------------
		PdesTime DeadlockDetectionSimulationManager::FindMinimumTimestampPerCluster(UInt32 ClusterID)
		{
			//printf("FindMinimumTimestampPerCluster(Int32 ClusterID): %d\n", ClusterID);
			register Int32 index = -2;
			PdesTime timestamp = 0;
			PdesTime minimum = -1;

			Cluster *cluster = clusters[ClusterID];

			pri_queue_lc *localQueue = cluster->GetLocalPriorityQueue();
			PdesTime pri;

			if(priq_top_lc(localQueue, &pri) != null)
			{
				//if(timestamp < minimum)
				{
					minimum = pri;
					//std::cout << "ClusterID::FindMinimumTimestampPerCluster1 " << ClusterID << " timestamp " << timestamp << std::endl;
					index = -1;
				}
			}

			for(register Int32 i = 0 ; i < cluster->GetInputChannelCount() ; i++)
			//Int32 initIndex = cluster->GetInputChannelCount() - 1;
			//for(register Int32 i = initIndex ; i != -1 ; --i)
			{
				DeadlockDetectionPriorityQueue *inputQueue = cluster->GetInputChannelByIndex(i);
				if(inputQueue == null)
					continue;

				if(inputQueue->blocked)
					continue;

				//if(inputQueue->PeekInternal(timestamp) == null)
				//	continue;
				EventInterface *e;

				if(inputQueue->peeked_event != null)
					e = inputQueue->peeked_event;
				else
				{
					RingBuffer<EventInterface *> *rb = inputQueue->rb;
					if(rb->GetCount() == 0)
						continue;

					e = rb->Dequeue();
					inputQueue->peeked_event = e;
				}


				timestamp = e->GetReceiveTimeLc();

				//std::cout << "ClusterID::FindMinimumTimestampPerCluster2 ClusterID " << ClusterID << " timestamp " << timestamp << std::endl;

				///if(i == initIndex && index == -2)
                if(i == 0 && index == -2)
				{
					minimum = timestamp;
					index = i;
					continue;
				}
				else if(timestamp < minimum)
				{
					minimum = timestamp;
					index = i;
				}
			}

			//std::cout << "index: " << index << std::endl;

			if(index == -2)
			{
				cluster->SetMinimumTimestamp(-1);
				cluster->SetMinimumTimestampQueue(null);
			}
			else if(index == -1)
			{
				cluster->SetMinimumTimestamp(minimum);
				cluster->SetMinimumTimestampQueue(null);
			}
			//else if(index == -3)
			//{
			//	cluster->SetMinimumTimestamp(minimum);
			//	cluster->SetMinimumTimestampQueue(cluster->deltaQueue);
			//}
			else
			{
                //std::cout << "ClusterID::FindMinimumTimestampPerCluster3 " << cluster->GetInputChannelByIndex(index) << std::endl;
				cluster->SetMinimumTimestamp(minimum);
				//cluster->SetMinimumTimestampQueue(cluster->GetLocalPriorityQueue());
				cluster->SetMinimumTimestampQueue(cluster->GetInputChannelByIndex(index));
			}
			/*if(index >= 0)
			{
                RingBuffer<EventInterface *> *rb  = cluster->GetMinimumTimestampQueue()->rb;
                printf("FindMinimumTimestampPerCluster::minimum: %d index: %d cluster %d count %d\n", (Int32)minimum, index, ClusterID, rb->GetCount());
			}*/

			return minimum;

		}
		//----------------------------------------------------
		void *DeadlockDetectionSimulationManager::Wrapper_To_Call_Controller(void *pt2Object)
		{
			// explicitly cast to a pointer to DeadlockDetectionSimulationManager
			DeadlockDetectionSimulationManager *mySelf = (DeadlockDetectionSimulationManager *)pt2Object;

			// call member
			mySelf->Controller();

			return pt2Object;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::OnDisposePriorityQueue(EventInterface *e)
		{
			if(e == null)
				return;

			EventInterface *_e = (EventInterface *)e;
			if(_e->GetEventType() == NormalMessage || _e->GetEventType() == AppNullMessage)
			{
				LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(_e->GetReceiverReference());
				if(receiver != null)
					if(_e->deletable)
						receiver->DeallocateEvent(_e);
			}
			else
				delete _e;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::RegisterOnPartitioningCompletionCallback(OnPartitioningCompletion callback, Object *state)
		{
			on_partitioning_completion_callback = callback;
			on_partitioning_completion_state = state;
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::AlarmShape(Int32 rows)
		{
			register Int32 n = rows, c, k, space = 1;

			//printf("Enter number of rows\n");
			//scanf("%d", &n);

			space = n - 1;

			mtx->Lock();

			for(k = 1; k <= n ; k++)
			{
				for(c = 1; c <= space ; c++)
					printf(" ");

				space--;

				for(c = 1 ; c <= 2*k-1 ; c++)
					printf("*");

				printf("\n");
			}

			space = 1;

			for(k = 1 ; k <= n - 1 ; k++)
			{
				for(c = 1 ; c <= space ; c++)
					printf(" ");

				space++;

				for(c = 1 ; c <= 2 *(n-k)-1 ; c++)
					printf("*");

				printf("\n");
			}

			mtx->Unlock();

			return ;
		}
        //----------------------------------------------------
        void DeadlockDetectionSimulationManager::GenerateDotGraph(const String &filename)
        {
            if(clusters.Size() == 0)
                return;
            // dot -Tpdf partitioning.dot -o partitioning.pdf
            // dot -Tsvg partitioning.dot -o partitioning.svg
            std::cout << "DeadlockDetectionSimulationManager::GenerateDotGraph() filename " << filename.get_BaseStream() << std::endl;

            ofstream *dotFile = new ofstream(filename.get_BaseStream(), ios::out);

            if(!dotFile->is_open())
            {
                delete dotFile;
                throw IOException("Could not open the file " + filename);
            }

            std::stringstream ss;
            ss << "digraph g\n{\n\tforcelabels=true;\n";
            ss << "\tnode [margin=0 fontcolor=red fontsize=20 width=0.5 shape=box style=filled];\n";

            for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
            {
                Vector<LogicalProcess *> *partiotion = partitions->GetPartitionByIndex(i);
				Cluster *cluster = clusters[i];
                //ss << "a [label="Birth of George Washington", xlabel="See also: American Revolution"];";
                //ss << "\t" <<  "n" << targetID << " [label=\"" << targetName << "\"];" << "\n";
                for(register UInt32 j = 0 ; j < clusters.Size() ; j++)
                    if(cluster->GetDestinationClusterChannelByID(j) != null)
                        ss << "\t" << "p" << i << " -> " << "p" << j << ";" << "\n";
            }

            ss << "}";

            *dotFile << ss.str();

            dotFile->flush();
            dotFile->close();
            delete dotFile;
        }
		//----------------------------------------------------
		/*void DeadlockDetectionSimulationManager::SendNullMassage(UInt32 currentCluster)
		{
			Cluster *srcCluster = clusters[currentCluster];

			UInt32 destClusters = srcCluster->_destinationClusters.Size();
			DeadlockDetectionPriorityQueue *destPQ;
			UInt64 simTime = srcCluster->GetLocaltime();
			EventInterface *newEvent;
			RingBuffer<EventInterface *> *rb;

			for(register Int32 i = destClusters - 1 ; i != -1 ; i--)
			{
				//std::cout << i << std::endl;
				destPQ = srcCluster->_destinationClusters[i];

				null_message_counter.Increment();
				destPQ->mutex->Lock();
				rb = destPQ->rb;
				if(rb->GetCount() == 0)
				{
					//std::cout << "Sending Null Message SourceCluster " << currentCluster  << " DestinationCluster " << i << std::endl;
					newEvent = new EventInterface();
					newEvent->SetEventType(NullMessage);
					newEvent->SetSendTime(simTime);
					//Int32 lookahead = 0;
					newEvent->SetReceiveTime(simTime); //? Take the lookahahead here from the programmer.
					rb->Enqueue(newEvent);

					//mtx->Lock(); printf("blocked count=0 and global_deadlock_counter: %d for Cluster: %d\n", global_deadlock_counter.get_Count(), GetOwnerClusterID()); mtx->Unlock();
					global_deadlock_counter.Increment();///
					SetBlockedPriorityQueue(destPQ->GetOwnerClusterID(), null);
					destPQ->blocked = false;
					destPQ->queueNotEmpty->Signal();

					null_message_counter.Increment();
				}
				destPQ->mutex->Unlock();
			}
		}
		//----------------------------------------------------
		void DeadlockDetectionSimulationManager::HandleNullMassage(EventInterface *e, UInt32 currentCluster)
		{
			delete e;

			SendNullMassage(currentCluster);
		}*/
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

