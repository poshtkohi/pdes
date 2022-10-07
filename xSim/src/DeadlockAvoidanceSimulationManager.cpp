/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "DeadlockAvoidanceSimulationManager.h"


// Stores the number of allocated logical processes.
extern UInt32 processCounter;
// Stores the allocated logical processes.
// We use an array to store the references to increase the access performance.
extern Vector<LogicalProcess *> *logicalProcesses;

LockFreeCounter global_termination_detection_counter = LockFreeCounter();
Mutex *global_mutex_termination_controller = new Mutex();
ConditionVariable *global_waitcv_termination_controller = new ConditionVariable(global_mutex_termination_controller);
bool global_termination_met = false;

//extern Mutex *mtx;
//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		DeadlockAvoidanceSimulationManager::Cluster::Cluster(UInt32 ClusterID, UInt32 numberOfClusters, UInt32 channelCapacity, SimulationManagerInterface *managerInstance)
		{
			localQueue = null;
			numberOfProcessedEvents = 0;
			localTime = 0;
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
			onBlock.OwnerClusterID = ClusterID;
			onBlock.onBlock = DeadlockAvoidanceSimulationManager::OnBlock;
			onBlock.state = managerInstance;
			localQueue = priq_new(channelCapacity); ///
			destinationClusters.Reserve(numberOfClusters);
			for(register UInt32 i = 0 ; i < numberOfClusters ; i++)
				destinationClusters[i] = null;
			destinationClustersCount = 0;
			clusterID = ClusterID;
			terminationHasRequested = false;
			terminated = false;
			workingInputChannelCount = 0;
			blockingCount = 0;
			blockedPriorityQueue = null;
			disposed = false;
			terminationCounter = 0;
			eventCounter = LockFreeCounter(0);
			nullMessageCounter = 0;

			tNull = tLastNull = 0;
			minNull = minNormal = LLONG_MAX;
/*#if defined __Delta_Enabled__
			deltaQueue = new DeadlockAvoidancePriorityQueue(channelCapacity, false);
			currentDeltaTimestamp = -1;
			deltaCounter = 0;
			isRecoveryFromDeltaQueue = false;
#endif*/
		}
		//----------------------------------------------------
		DeadlockAvoidanceSimulationManager::Cluster::~Cluster()
		{
			//printf("~Cluster\n");//
			if(!disposed)
			{
				if(localQueue != null)
				{
					ps2_priority pri;
					EventInterface *e;
					while( ( e = (EventInterface *)priq_pop(localQueue, &pri) ) != null)
						//delete e;
						DeadlockAvoidanceSimulationManager::OnDisposePriorityQueue(e);
					priq_delete(localQueue);
				}

				for(register UInt32 i = 0 ; i < inputQueues.Size() ; i++)
					if(inputQueues[i] != null)
						delete inputQueues[i];

				disposed = true;
			}
		}
		//----------------------------------------------------
		DeadlockAvoidancePriorityQueue *DeadlockAvoidanceSimulationManager::Cluster::AddInputChannel()
		{
			if(disposed)
				throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			DeadlockAvoidancePriorityQueue *inputQueue = new DeadlockAvoidancePriorityQueue(channelCapacity, DeadlockAvoidanceSimulationManager::OnDisposePriorityQueue, &onBlock);
			inputQueue->SetOwnerClusterID(GetClusterID());
			inputQueue->SetSimulationManager(manager);
			inputQueues.PushBack(inputQueue);
			workingInputChannelCount++;
			return inputQueue;
		}
		//----------------------------------------------------
		UInt32 DeadlockAvoidanceSimulationManager::Cluster::GetInputChannelCount()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");

			return inputQueues.Size();
		}
		//----------------------------------------------------
		DeadlockAvoidancePriorityQueue *DeadlockAvoidanceSimulationManager::Cluster::GetInputChannelByIndex(UInt32 index)
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			//if(index < 0 || index > inputQueues.Size() - 1)
			//	throw ArgumentOutOfRangeException("index", "index must be greater than zero and be less than GetInputChannelCount()");
			return inputQueues[index];
		}
		//----------------------------------------------------
		bool DeadlockAvoidanceSimulationManager::Cluster::ContainsDestinationCluster(UInt32 destionationClusterID)
		{
			if(disposed)
				throw ObjectDisposedException("Cluster", "The Cluster has been disposed");

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
		void DeadlockAvoidanceSimulationManager::Cluster::AddDestionationCluster(UInt32 destionationClusterID, DeadlockAvoidancePriorityQueue *destinationChannel)
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
		UInt32 DeadlockAvoidanceSimulationManager::Cluster::GetDestinationClusterCount()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			return destinationClustersCount;
		}
		//----------------------------------------------------
		DeadlockAvoidancePriorityQueue *DeadlockAvoidanceSimulationManager::Cluster::GetDestinationClusterChannelByID(UInt32 destionationClusterID)
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			//if(destionationClusterID < 0 || destionationClusterID > destinationClusters.Size() - 1)
			//	throw ArgumentOutOfRangeException("destionationClusterID", "destionationClusterID must be greater than or equal to zero and be less than numberOfClusters-1");
			return destinationClusters[destionationClusterID];
		}
		//----------------------------------------------------
		pri_queue *DeadlockAvoidanceSimulationManager::Cluster::GetLocalPriorityQueue()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			return localQueue;
		}
		//----------------------------------------------------
		UInt32 DeadlockAvoidanceSimulationManager::Cluster::GetClusterID()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			return clusterID;
		}
		//----------------------------------------------------
		UInt64 DeadlockAvoidanceSimulationManager::Cluster::GetLocaltime()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			return localTime;
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::Cluster::SetLocaltime(UInt64 newTime)
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			localTime = newTime;
		}
		//----------------------------------------------------
		bool DeadlockAvoidanceSimulationManager::Cluster::GetTerminated()
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");

			return terminated;
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::Cluster::SetTerminated(bool val)
		{
			//if(disposed)
			//	throw ObjectDisposedException("Cluster", "The Cluster has been disposed");
			terminated = val;
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::Cluster::RemoveInputChannelByIndex(UInt32 index)
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
		DeadlockAvoidanceSimulationManager::DeadlockAvoidanceSimulationManager(UInt32 numOfLogicalProcessors, UInt32 channelCapacity, Partitioner *partitioner)
		{
			objects = null;
			partitions = null;
			controller = null;
			mutex_completion = null;
			waitcv_completion = null;
			mutex_simulation_phases = null;
			waitcv_simulation_phases = null;
			if(numOfLogicalProcessors == 0)
				throw ArgumentOutOfRangeException("numOfLogicalProcessors", "numOfLogicalProcessors is equal to zero");
			if(channelCapacity == 0)
				throw ArgumentOutOfRangeException("channelCapacity", "channelCapacity is equal to zero");
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
			simulation_phases_met = false;
			on_partitioning_completion_callback = null;
			on_partitioning_completion_state = null;
			//clusterEvents = null;
			controllerThreadTerminationRequested = false;
			disposed = false;
		}
		//----------------------------------------------------
		DeadlockAvoidanceSimulationManager::~DeadlockAvoidanceSimulationManager()
		{
			//printf("~DeadlockAvoidanceSimulationManager\n");
			Dispose();
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::Dispose()
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

					if(controller != null)
						delete controller;
					/*if(clusterEvents != null)
					{
						UInt32 numberOfClusterEvents = nearestPower(clusters.Size());
						for(register UInt32 i = 0 ; i < numberOfClusterEvents ; i++)
						if(clusterEvents[i] != null)
						delete clusterEvents[i];
						delete clusterEvents;
					}*/

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

					pthread_spin_destroy(&lock);
				}

				if(objects != null)
					delete objects;

				disposed = true;
			}
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::RegisterLogicalProcess(LogicalProcess *lp)
		{
			if(disposed)
				throw ObjectDisposedException("DeadlockAvoidanceSimulationManager", "The DeadlockAvoidanceSimulationManager has been disposed");
			if(lp == null)
				throw ArgumentNullException("lp", "lp is null");
			objects->PushBack(lp);
			lp->SetSimulationManager(this);
		}
		//----------------------------------------------------
		UInt32 DeadlockAvoidanceSimulationManager::GetNumberOfLogicalProcesses()
		{
			//if(disposed)
			//	throw ObjectDisposedException("DeadlockAvoidanceSimulationManager", "The DeadlockAvoidanceSimulationManager has been disposed");

			return objects->Size();
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::Run(UInt64 simulateUntil)
		{
			if(disposed)
				throw ObjectDisposedException("DeadlockAvoidanceSimulationManager", "The DeadlockAvoidanceSimulationManager has been disposed");

			if(simulateUntil <= 0)
				throw ArgumentOutOfRangeException("simulateUntil", "simulateUntil is less than or equal to zero.");

			if(GetNumberOfLogicalProcesses() == 0)
				throw ArgumentOutOfRangeException("GetNumberOfLogicalProcesses", "You must register at least one logical process instance");

			this->simulateUntil = simulateUntil;

			if(!started)
			{
				started = true;

				cout << "\n\t PS2 v2.2.1 (Many-Core Release)\n\t Conservative Protocol with Deadlock Avoidance (Null Messages)\n";
				cout << "\t Parvicursor Simulation System\n";
				cout << "\t Copyright (c) 2004-" << DateTime::get_Now().get_Year() << " by Shahed University,\n";
				cout << "\t ALL RIGHTS RESERVED\n\n" << endl;
				cout << "Initializing parallel simulation environment ..." << endl;

				PrepareForParallelSimulation();
			}
			else
				throw ThreadStateException("The simulation has already been started");
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::WaitOnManagerCompletion()
		{
			if(disposed)
				throw ObjectDisposedException("DeadlockAvoidanceSimulationManager", "The DeadlockAvoidanceSimulationManager has been disposed");

			if(!started)
				throw InvalidOperationException("Run() method must first be called");

			mutex_completion->Lock();
			while( completedThreadCount != clusters.Size() )
				waitcv_completion->Wait();
			mutex_completion->Unlock();

			cout << "Finalizing simulation ...\n";
			UInt64 numberOfProcessedEvents = 0;
			for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
				numberOfProcessedEvents += clusters[i]->numberOfProcessedEvents;

			UInt64 numberOfNullMessages = 0;
			for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
				numberOfNullMessages += clusters[i]->nullMessageCounter;

			double d1, d2;
			d1 = (double)start.tv_sec + 1e-6*((double)start.tv_usec);
			d2 = (double)stop.tv_sec + 1e-6*((double)stop.tv_usec);
			// return result in seconds
			Double totalSimulationTime = d2 - d1;

			UInt32 density = 0;
			if(totalSimulationTime != 0)
				density = numberOfProcessedEvents / totalSimulationTime;

			cout << "Simulation completed (" << numberOfProcessedEvents << " events in "
				<< totalSimulationTime << " secs, "
				<< density << " events/sec, ";
			density = 0;
			if(totalSimulationTime != 0)
				density = numberOfNullMessages / totalSimulationTime;
				//density = numberOfDeadlocks / totalSimulationTime;

			cout << numberOfNullMessages << " null-messages, " << density << " null-messages/sec executed on " << clusters.Size() << " logical processors with " << Environment::get_ProcessorCount() << " physical cores).\n" << endl;

			for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
				cout << "Number of (blocking states, processed events) in Cluster " << i << " is (" << clusters[i]->blockingCount << ", " << clusters[i]->numberOfProcessedEvents << ")" << ".\n";

			cout << endl;

			usage u;
			ResourceProfiler::GetResourceUsage(&u);
			ResourceProfiler::PrintResourceUsage(&u);

			///Dispose(); ///

			return ;
		}
		//----------------------------------------------------
		LogicalProcess *DeadlockAvoidanceSimulationManager::GetLogicalProcessById(UInt32 id)
		{
			//if(id < 0)
			//	throw ArgumentOutOfRangeException("id", "id is less than zero");

			return (*logicalProcesses)[id];
		}
		//----------------------------------------------------
		UInt64 DeadlockAvoidanceSimulationManager::GetSimulationTime()
		{
			//if(disposed)
			//	throw ObjectDisposedException("DeadlockAvoidanceSimulationManager", "The DeadlockAvoidanceSimulationManager has been disposed");

			// Here, we must consider a lock-based or lock-free implementation for this method.

			return simulationTime;
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::SetSimulationTime(UInt64 newTime)
		{
			//if(disposed)
			//	throw ObjectDisposedException("DeadlockAvoidanceSimulationManager", "The DeadlockAvoidanceSimulationManager has been disposed");

			// Here, we must consider a lock-based or lock-free implementation for this method.
			simulationTime = newTime;
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::PrepareForParallelSimulation()
		{
			if(disposed)
				throw ObjectDisposedException("DeadlockAvoidanceSimulationManager", "The DeadlockAvoidanceSimulationManager has been disposed");

			if(hasPreparedForParallelSimulation)
				return ;

			// Here, we must consider NUMA-aware thread programming styles.
			//UInt32 originalNumberOfCores = Environment::get_ProcessorCount();
			//UInt32 numberOfCores = originalNumberOfCores;
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
			for(UInt32 i = 0 ; i < objects->Size() ; i++)
				objs->push_back((*objects)[i]);
			partitions = partitioner->Partition(objs, numberOfCores);*/



			/*printf("Before partitioning\n");
			printf("logicalProcesses->Size(): %d\n", logicalProcesses->Size());
			for(register UInt32 i = 0 ; i < logicalProcesses->Size() ; i++)
			{
				LogicalProcess *lp = (*logicalProcesses)[i];
				printf("LP %d is %s output LPs are ", i, lp->GetName().get_BaseStream());
				for(register UInt32 j = 0 ; j < lp->GetOutputLpCount() ; j++)
					printf(" %s", LogicalProcess::GetLogicalProcessById(lp->GetOutputLpId(j))->GetName().get_BaseStream());
				printf("\n");
			}
			exit(0);*/

			partitions = partitioner->Partition(objects, numberOfCores);

			///if(numberOfCores > originalNumberOfCores)
			///	throw ArgumentOutOfRangeException("numberOfPartitions", "The updated numberOfPartitions is greater than the requested value by the simulation kernel");

			global_termination_detection_counter = LockFreeCounter(numberOfCores);
			threadCounter = 0;
			/*UInt32 numberOfClusterEvents = nearestPower(numberOfCores);
			clusterEvents = new EventInterface *[numberOfClusterEvents];
			for(register UInt32 i = 0 ; i < numberOfClusterEvents ; i++)
					clusterEvents[i] = new EventInterface();*/

			for(register UInt32 i = 0 ; i < numberOfCores ; i++)
				clusters.PushBack(new Cluster(i, numberOfCores, channelCapacity, this));


			if(partitions == null)
				throw ArgumentNullException("Partitioner.Partition()", "Partitioner.Partition() returned a null value");

			for(register UInt32 currentClusterID = 0 ; currentClusterID < partitions->GetNumberOfPartitions() ; currentClusterID++)
			{
				Vector<LogicalProcess *> *partition = partitions->GetPartitionByIndex(currentClusterID);

#if defined __Parvicursor_xSim_Debug_Enable__
				cout << "============================================================" << endl;
				cout << "In Cluster: " << currentClusterID << endl;
#endif

				for(register UInt32 i = 0 ; i < partition->Size() ; i++)
				{
					// Source
					LogicalProcess *src = (*partition)[i];
#if defined __Parvicursor_xSim_Debug_Enable__
					cout << "LP Name: " << src->GetName().get_BaseStream() << " GetOutputLpCount(): " << src->GetOutputLpCount() << endl;
#endif
					for(register UInt32 j = 0 ; j < src->GetOutputLpCount() ; j++)
					{
						// Destination
						LogicalProcess *dest = GetLogicalProcessById(src->GetOutputLpId(j));
						UInt32 destClusterID = dest->GetOwnerClusterID();
#if defined __Parvicursor_xSim_Debug_Enable__
						cout << "currentClusterID: " << currentClusterID << " destClusterID: " << destClusterID << endl;
#endif
						if(currentClusterID != destClusterID)
						{
							if(!clusters[currentClusterID]->ContainsDestinationCluster(destClusterID))
							{
								//cout << "Hello !" << " currentClusterID: " << currentClusterID << " destClusterID: " << destClusterID << endl;
								DeadlockAvoidancePriorityQueue *destInputChannel = clusters[destClusterID]->AddInputChannel();
								clusters[currentClusterID]->AddDestionationCluster(destClusterID, destInputChannel);
							}
						}
					}
				}

#if defined __Parvicursor_xSim_Debug_Enable__
				cout << "GetDestinationClusterCount(): " << clusters[currentClusterID]->GetDestinationClusterCount() << endl;

				cout << "Destination channels: ";
				for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
					cout << clusters[currentClusterID]->GetDestinationClusterChannelByID(i) << ", ";
				cout << endl;
#endif
			}

#if defined __Parvicursor_xSim_Debug_Enable__
			// Prints the information of clusters
			for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
			{
				std::cout << "------------------------------------------------\n";
				Vector<LogicalProcess *> *partiotion= partitions->GetPartitionByIndex(i);
				Cluster *cluster = clusters[i];
				cout << "Cluster Information " << i << "\n";
				cout << "Number of Input Channels: " << cluster->GetInputChannelCount() << "\n" ;
				cout << "Number of Output Channels (Destination Clusters): " << cluster->GetDestinationClusterCount() << "\n" ;
				if(cluster->GetDestinationClusterCount() > 0)
				{
					cout << "Destination Clusters' ID: ";
					for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
						if(cluster->GetDestinationClusterChannelByID(i) != null)
							cout << i << ", ";
					cout << "\n";
				}

				if(partiotion->Size() > 0)
				{
					std::cout << "Logical Processes: ";
					for(register UInt32 i = 0 ; i < partiotion->Size() ; i++)
						std::cout << (*partiotion)[i]->GetName().get_BaseStream() << "(" << (*partiotion)[i]->GetID() << "), ";
					std::cout << "\n";
				}
				std::cout << endl;
			}

			std::cout << "------------------------------------------------\n";
#endif
			if(on_partitioning_completion_callback != null)
				on_partitioning_completion_callback(on_partitioning_completion_state);

			//exit(0);
			// ***************************** Thread Creation ************************* //
			mutex_completion = new Mutex();
			waitcv_completion = new ConditionVariable(mutex_completion);

			mutex_simulation_phases = new Mutex();
			waitcv_simulation_phases = new ConditionVariable(mutex_simulation_phases);

			controller = new Thread(Wrapper_To_Call_Controller, (void *)this);
			controller->Start();
			controller->SetDetached();

			pthread_spin_init(&lock, pshared);

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
		inline Int64 CompueteMimimum(Vector<pair<EventInterface *, Int32> > *array)
		{
			pair<EventInterface *, Int32> p = (*array)[0];

			UInt64 minimum = p.first->GetReceiveTime();
			UInt64 time;

			for(register Int32 i = 1 ; i < array->Size() ; i++)
			{
				p = (*array)[i];
				time = p.first->GetReceiveTime();
				if(time < minimum)
					minimum = time;
			}

			return minimum;
			/*
				SET Max to array[0]
				FOR i = 1 to array length - 1
					IF array[i] > Max THEN
						SET Max to array[i]
					ENDIF
				ENDFOR
				PRINT Max
			*/
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::Worker()
		{
			//Int32 ClusterID = (Int32)threadCounter.get_Count_And_Increment();
			UInt32 ClusterID;
			//mutex_threadCounter->Lock();
			pthread_spin_lock(&lock);
			ClusterID = threadCounter;
			threadCounter++;
			pthread_spin_unlock(&lock);
			//mutex_threadCounter->Unlock();
			//cout << "threadCounter.get_Count(): " << threadCounter.get_Count() << endl;
			Cluster *cluster = clusters[ClusterID];
			pri_queue *localQueue = cluster->GetLocalPriorityQueue();
			Vector<LogicalProcess *> *partiotion= partitions->GetPartitionByIndex(ClusterID);
			UInt64 localSimulationTime = 0;

#if defined __Parvicursor_xSim_Debug_Enable__
			// ************************** Prints Cluster Information ********************** //
			mtx->Lock();
			cout << "------------------------------------------------\n";
			cout << "I'm thread working on cluster " << ClusterID << "\n";
			cout << "Cluster Information \n";
			cout << "Number of Input Channels: " << cluster->GetInputChannelCount() << "\n" ;
			cout << "Number of Output Channels (Destination Clusters): " << cluster->GetDestinationClusterCount() << "\n" ;
			if(cluster->GetDestinationClusterCount() > 0)
			{
				cout << "Destination Clusters' ID: ";
				for(register UInt32 i = 0 ; i < clusters.Size() ; i++)
					if(cluster->GetDestinationClusterChannelByID(i) != null)
						cout << i << ", ";
				cout << "\n";
			}

			if(partiotion->size() > 0)
			{
				cout << "Logical Processes: ";
				for(register UInt32 i = 0 ; i < partiotion->size() ; i++)
					cout << (*partiotion)[i]->GetName().get_BaseStream() << ", ";
				cout << "\n";
			}

			cout << endl;
			mtx->Unlock();
#endif
			// ******************* Performs Conservative Simulation ******************** //

			if(ClusterID == 0)
			{
				mtx->Lock(); cout << "PS2 Running ..." << endl; mtx->Unlock();
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

			if(ClusterID == clusters.Size() - 1)
				xParvicursor_gettimeofday(&start, null);
			// ********************************** End Barrier Phase ************************************** //
			UInt64 timestamp = -1;
			EventInterface *nextEvent = null;
			EventInterface *nextEventMinimum = null;
			DeadlockAvoidancePriorityQueue *inputQueue  = null;
			EventType eventType;
			LogicalProcess *receiver = null;
			register Int32 initIndex = cluster->GetInputChannelCount() - 1;
			register Int32 i = initIndex;
			ps2_priority pri;

			// localQueue and input channels
			Vector<pair<EventInterface *, Int32> > *NormalMessages = new Vector<pair<EventInterface *, Int32> >(1 + cluster->GetInputChannelCount());

			//UInt64 minNormal = 0;
			//UInt64 minNull = 0;
			//UInt64 tNull = 0;
			//UInt64 tLastNull = 0;
			bool global_counter_decremented = false;


			///cout << "localQueue: " << localQueue << endl;
			// We must execute the events from localQueue if we are at the beginning of the simulation
			if(localSimulationTime == 0)
			{
				nextEvent = (EventInterface *)priq_top(localQueue, &pri);
				if(nextEvent != null)
				{
					nextEvent = (EventInterface *)priq_pop(localQueue, &pri);
					timestamp = pri.timestamp;
					cluster->minNormal = timestamp;
					///mtx->Lock(); cout << "x1 Cluster " << ClusterID << " localSimulationTime " << localSimulationTime << " minimum " << timestamp << endl; mtx->Unlock();
					localSimulationTime = timestamp;
					cluster->SetLocaltime(localSimulationTime);
					LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
					receiver->SetSimulationTime(timestamp);
					//receiver->pq = localQueue;
					receiver->e = nextEvent;
					receiver->ExecuteProcess();
					if(nextEvent->deletable)
                        receiver->DeallocateEvent(nextEvent);///

					// Send null messages here ?

					Long counter = cluster->eventCounter.Decrement_And_get_Count();

					if(counter == 0)
					{
						global_counter_decremented = true;
						if(global_termination_detection_counter.Decrement_And_get_Count() == 0)
						{
							std::cout << "A global termination point was detected by the simulation manager at LVT " << localSimulationTime << " in Cluster " << ClusterID << endl;
							goto EndWhile1;
						}

						//std::cout << "global_termination_detection_counter " << global_termination_detection_counter.get_Count() << " in Cluster " << ClusterID << endl;
					}

					//mtx->Lock(); std::cout << "GetNextEvent() destCluster " << ClusterID << " counter " << counter << endl; mtx->Unlock();
				}

				// here send primary null messages.
			}

			///mtx->Lock(); std::cout << "Initial Cluster " << ClusterID <<  " localSimulationTime " << localSimulationTime << endl; mtx->Unlock();///

			//EventInterface **inputEvents = new EventInterface*[1 + cluster->GetInputChannelCount()];

			//while(true){}

BeginWhile:
			while(localSimulationTime < simulateUntil  /*&& tNull < simulateUntil*/)
			{
				cluster->minNormal = LLONG_MAX;
				cluster->minNull = LLONG_MAX;
				// -------------------- Get Normal and Null Messages -------------------------- //
				// Local Queue
				//mtx->Lock();std::cout << "Cluster " << ClusterID << " Local Queue localSimulationTime " << localSimulationTime << endl;mtx->Unlock();///
				nextEvent = (EventInterface *)priq_top(localQueue, &pri);
				if(nextEvent != null)
				{
					pair<EventInterface *, Int32> p;
					p.first = nextEvent;
					p.second = -1;
					cluster->minNormal = nextEvent->GetReceiveTime();
					NormalMessages->UnsafePushBack(p);
				}
				else if (cluster->GetInputChannelCount() == 0)
					break;
				

				// Input Queues
				//for(i = 0 ; i < cluster->GetInputChannelCount() ; i++)
				for(i = cluster->GetInputChannelCount() - 1 ; i != -1 ; i--)
				{
					///mtx->Lock();std::cout << "1 Cluster " << ClusterID << " InputQueue Queue i " << i << endl;mtx->Unlock();///
					DeadlockAvoidancePriorityQueue *inputQueue = cluster->GetInputChannelByIndex(i);
					///mtx->Lock();std::cout << "2 Cluster inputQueue " << inputQueue <<endl;mtx->Unlock();///

					nextEvent = inputQueue->Peek();

					timestamp = nextEvent->GetReceiveTime();
					eventType = nextEvent->GetEventType();

					if(eventType == NormalMessage)
					{
						pair<EventInterface *, Int32> p;
						p.first = nextEvent;
						p.second = i;

						if(timestamp < cluster->minNormal)
						{
							cluster->minNormal = timestamp;
							NormalMessages->Reset();
							NormalMessages->UnsafePushBack(p);
						}
						else if(timestamp == cluster->minNormal)
							NormalMessages->UnsafePushBack(p);
					}
					else if(eventType == NullMessage)
					{
						if(timestamp < cluster->minNull)
							cluster->minNull = timestamp;

						nextEvent = inputQueue->Dequeue();
						delete nextEvent;
					}
					else if(eventType == RequestToRemoveTheChannelMessage)
					{
						nextEvent = inputQueue->Dequeue();
						cluster->RemoveInputChannelByIndex(i);
						//initIndex = cluster->GetInputChannelCount() - 1;
						//mtx->Lock(); printf("Receiving RequestToRemoveTheChannelMessage1 in Cluster %d from Cluster %d timestamp %d GetLocaltime %d workingInputChannelCount %d index %d\n", ClusterID, nextEvent->GetSenderReference(), (Int32)timestamp, (Int32)cluster->GetLocaltime(), cluster->workingInputChannelCount, i); mtx->Unlock();
						delete nextEvent;
						NormalMessages->Reset();
						goto BeginWhile;
					}
				}

				// -------------------- Compute minNull and minNormal -------------------------- //

				//? about minNormal <= minNull
				// -------------------- Process all NormalMessages with minNormal if minNormal < minNull ----------- //
				if(cluster->minNormal < cluster->minNull)
				{
					//for(register Int32 n = 0 ; n < NormalMessages->Size() ; n++)
					for(register Int32 n = NormalMessages->Size() - 1 ; n != -1 ; n--)
					{
						pair<EventInterface *, Int32> p = (*NormalMessages)[n];
						nextEvent = p.first;
						timestamp = nextEvent->GetReceiveTime();
						if(p.second == -1)
							nextEvent = (EventInterface *)priq_pop(localQueue, &pri);
						else
							nextEvent = cluster->GetInputChannelByIndex(p.second)->Dequeue();

						localSimulationTime = timestamp;///?
						cluster->SetLocaltime(localSimulationTime);///?
						receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
						receiver->SetSimulationTime(localSimulationTime);
						receiver->e = nextEvent;
						receiver->ExecuteProcess();
						if(nextEvent->deletable)
							receiver->DeallocateEvent(nextEvent);

						Long counter = cluster->eventCounter.Decrement_And_get_Count();

						//mtx->Lock(); std::cout << "GetNextEvent() destCluster " << ClusterID << " counter " << counter << endl; mtx->Unlock();
						if(counter == 0)
						{
							if(global_counter_decremented)
							{
								if(global_termination_detection_counter.get_Count() == 0)
								{
									std::cout << "A global termination point was detected by the simulation manager at LVT " << localSimulationTime << " in Cluster " << ClusterID << endl;
									goto EndWhile1;
								}
							}
							else
							{
								global_counter_decremented = true;
								if(global_termination_detection_counter.Decrement_And_get_Count() == 0)
								{
									std::cout << "A global termination point was detected by the simulation manager at LVT " << localSimulationTime << " in Cluster " << ClusterID << endl;
									goto EndWhile1;
								}
							}
							//std::cout << "global_termination_detection_counter " << global_termination_detection_counter.get_Count() << " in Cluster " << ClusterID << endl;
						}
						else
						{
							if(global_counter_decremented)
							{
								global_termination_detection_counter.Increment();
								global_counter_decremented = false;
							}
						}
					}
				}
				// -------------------- Send Null Messages-------------------------------- //
				//SendNullMassage(ClusterID);

				NormalMessages->Reset();
			}

EndWhile1:

			if(!global_counter_decremented)
			{
				//cout << "global_counter_decremented" << endl;
				if(global_termination_detection_counter.Decrement_And_get_Count() == 0)
				{
					std::cout << "A global termination point was detected by the simulation manager at LVT " << localSimulationTime << " in Cluster " << ClusterID << endl;
					//goto EndWhile1;
				}
			}
			//printf("---------EndWhile1-------- in Cluster %d localSimulationTime: %d localQueueCount: %d eventCounter %d global_termination_detection_counter %d\n", ClusterID, (UInt32)localSimulationTime, localQueue->length, cluster->eventCounter.get_Count(), global_termination_detection_counter.get_Count()); ///
			/*for(int i = 0 ; i < clusters.Size(); i++)
				printf("Hello in Cluster %d localSimulationTime %d\n", i, clusters[i]->GetLocaltime()); ///*/

			/*if(!global_counter_decremented)
			{
				if(global_termination_detection_counter.Decrement_And_get_Count() == 0)
				{
					std::cout << "A global termination point was detected by the simulation manager at LVT " << localSimulationTime << " in Cluster " << ClusterID << endl;
				}
				global_counter_decremented = true;
			}*/
			for(i = cluster->_destinationClusters.Size() - 1 ; i != -1 ; i--)
			//for(register Int32 i = 0 ; i < cluster->_destinationClusters.Size() ; i++)
			{
				DeadlockAvoidancePriorityQueue *outputChannel = cluster->_destinationClusters[i];
				//printf("ClusterID: %d DestinatonCluster: %d Count: %d\n", ClusterID, outputChannel->GetOwnerClusterID(), outputChannel->GetCount());
				EventInterface *newEvent = new EventInterface();
				UInt64 time = cluster->GetLocaltime();//cluster->GetLatestSendTime() + 1;
				newEvent->SetEventType(RequestToRemoveTheChannelMessage);
				newEvent->SetReceiveTime(time);
				newEvent->SetSendTime(time);
				outputChannel->Enqueue(newEvent);
				//mtx->Lock(); printf("Sending RequestToRemoveTheChannelMessage from Cluster %d to Cluster %d time %d GetLocaltime %d\n", ClusterID, outputChannel->GetOwnerClusterID(), (UInt32)time, (UInt32)cluster->GetLocaltime()); mtx->Unlock();
			}

//EndWhile2:
			//printf("---------EndWhile2-------- in Cluster %d localSimulationTime: %d localQueueCount: %d\n", ClusterID, (UInt32)localSimulationTime, localQueue->length); ///
			cluster->SetTerminated(true);

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


			global_mutex_termination_controller->Lock();
			controllerThreadTerminationRequested = true;
			global_termination_met = true;
			global_waitcv_termination_controller->Signal();
			global_mutex_termination_controller->Unlock();

			if(ClusterID == clusters.Size() - 1)
				xParvicursor_gettimeofday(&stop, null);

			for(register UInt32 i = 0 ; i < partiotion->Size() ; i++)
				(*partiotion)[i]->Finalize();

			mutex_completion->Lock();
			completedThreadCount++;
			waitcv_completion->Signal();
			mutex_completion->Unlock();

			delete NormalMessages;

		}
		//----------------------------------------------------
		void *DeadlockAvoidanceSimulationManager::Wrapper_To_Call_Worker(void *pt2Object)
		{
			// explicitly cast to a pointer to DeadlockAvoidanceSimulationManager
			DeadlockAvoidanceSimulationManager *mySelf = (DeadlockAvoidanceSimulationManager *)pt2Object;

			// call member
			mySelf->Worker();

			return pt2Object;
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::Controller()
		{
			global_mutex_termination_controller->Lock();
			while(!global_termination_met)
				global_waitcv_termination_controller->Wait();
			global_mutex_termination_controller->Unlock();

			//std::cout << "A global termination point was detected by the simulation controller thread." << endl;

			if(controllerThreadTerminationRequested)
				return;

			for(register Int32 clusterIndex = 0 ; clusterIndex < clusters.Size() ; clusterIndex++)
			{
				Cluster *cluster = clusters[clusterIndex];
				if(cluster->GetTerminated())
					continue;
				cluster->terminationHasRequested = true;
				for(register UInt32 i = cluster->GetInputChannelCount() - 1 ; i != -1 ; i--)
				{
					DeadlockAvoidancePriorityQueue *inputQueue = cluster->GetInputChannelByIndex(i);
					if(inputQueue == null)
						continue;
					inputQueue->mutex->Lock();
					if(inputQueue->blocked)
					{
						//printf("Sending TerminationHasRequestedMessage from Control Thread to Cluster %d\n", cluster->GetClusterID());
						inputQueue->blocked = false;
						EventInterface *newEvent = new EventInterface();
						UInt64 time = cluster->GetLocaltime();
						newEvent->SetEventType(TerminationHasRequestedMessage);
						newEvent->SetReceiveTime(time);
						newEvent->SetSendTime(time);
						inputQueue->rb->Enqueue(newEvent);
						inputQueue->queueNotEmpty->Signal();
						inputQueue->mutex->Unlock();
						break;
					}
					inputQueue->mutex->Unlock();
				}
			}
		}
		//----------------------------------------------------
		void *DeadlockAvoidanceSimulationManager::Wrapper_To_Call_Controller(void *pt2Object)
		{
			// explicitly cast to a pointer to DeadlockAvoidanceSimulationManager
			DeadlockAvoidanceSimulationManager *mySelf = (DeadlockAvoidanceSimulationManager *)pt2Object;

			// call member
			mySelf->Controller();

			return pt2Object;
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::OnDisposePriorityQueue(EventInterface *e)
		{
			if(e == null)
				return;

			if(e->GetEventType() == NormalMessage)
			{
				LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference());
				if(receiver != null)
					if(e->deletable)
						receiver->DeallocateEvent(e);
			}
			else
			{
				delete e;
			}
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::RegisterOnPartitioningCompletionCallback(OnPartitioningCompletion callback, Object *state)
		{
			on_partitioning_completion_callback = callback;
			on_partitioning_completion_state = state;
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::SendNullMassage(UInt32 currentCluster)
		{
			Cluster *cluster = clusters[currentCluster];
			cluster->blockingCount++;
			UInt64 lookahead = 1; // 0 or 1
			cluster->tNull = min(cluster->minNormal, cluster->minNull) + lookahead;
			if(cluster->tNull > cluster->tLastNull)
			{	
				//mtx->Lock();std::cout << "Cluster " << ClusterID << " tNull " << tNull << " tLastNull " << tLastNull << endl; mtx->Unlock();///
				//mtx->Lock();std::cout << "Cluster " << ClusterID << " tNull > tLastNull " << endl;mtx->Unlock();
				UInt64 sendTime = cluster->GetLocaltime();
				UInt64 recvTime = cluster->tNull;
				//for(register Int32 dest = 0 ; dest < cluster->_destinationClusters.Size() ; dest++)
				for(register Int32 dest = cluster->_destinationClusters.Size() - 1 ; dest != -1 ; dest--)
				{
					DeadlockAvoidancePriorityQueue *destPQ = cluster->_destinationClusters[dest];
					//mtx->Lock();std::cout << "Cluster " << ClusterID << " send null message to Cluster " << destPQ->GetOwnerClusterID() << " tNull " << tNull << endl;mtx->Unlock();///
					EventInterface *newEvent = new EventInterface();
					newEvent->SetEventType(NullMessage);
					//newEvent->SetSendTime(sendTime);
					newEvent->SetReceiveTime(recvTime);
					destPQ->Enqueue(newEvent); // if dest is empty, send a null-message
					cluster->nullMessageCounter++;
					///mtx->Lock();std::cout << "Cluster " << ClusterID << " recvTime " << recvTime << endl;mtx->Unlock();///
				}
				cluster->tLastNull = cluster->tNull;
			}
		}
		//----------------------------------------------------
		void DeadlockAvoidanceSimulationManager::OnBlock(Object *state, UInt32 OwnerClusterID)
		{
			DeadlockAvoidanceSimulationManager *manager = (DeadlockAvoidanceSimulationManager *)state;
			manager->SendNullMassage(OwnerClusterID);
		}
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//