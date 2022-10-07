/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "TimeWarpSimulationManager.h"
#include <../Samples/LockFree/LockFree/LockFreeCounter/LockFreeCounter.h>


// Stores the number of allocated logical processes.
extern UInt32 processCounter;
// Stores the allocated logical processes.
// We use an array to store the references to increase the access performance.
extern Vector<LogicalProcess *> *logicalProcesses;


LockFree::LockFreeCounter tw_global_termination_counter;
Mutex *tw_global_mutex_controller = new Mutex();
ConditionVariable *tw_global_waitcv_controller = new ConditionVariable(tw_global_mutex_controller);
bool tw_global_met = false;

Mutex *tw_mutex = new Mutex();

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        //----------------------------------------------------
		TimeWarpSimulationManager::TimeWarpSimulationManager(UInt32 numOfLogicalProcessors, Partitioner *partitioner)
		{
			objects = null;
			partitions = null;
			controller = null;
			mutex_completion = null;
			waitcv_completion = null;

			if(numOfLogicalProcessors == 0)
				throw ArgumentOutOfRangeException("numOfLogicalProcessors", "numOfLogicalProcessors is equal to zero");
			if(partitioner == null)
				throw ArgumentNullException("partitioner", "partitioner is null");

			this->numOfLogicalProcessors = numOfLogicalProcessors;
			objects = new Vector<LogicalProcess *>;
			completedThreadCount = 0;
			completedThreadCount_simulation_phases = 0;
			hasPreparedForParallelSimulation = false;
			controllerThreadTerminationRequested = false;
			started = false;
			this->partitioner = partitioner;
			on_partitioning_completion_callback = null;
			on_partitioning_completion_state = null;

			disposed = false;
		}
		//----------------------------------------------------
		TimeWarpSimulationManager::~TimeWarpSimulationManager()
		{
			Dispose();
		}
		//----------------------------------------------------
		void TimeWarpSimulationManager::Dispose()
		{
			if(!disposed)
			{
				if(hasPreparedForParallelSimulation)
				{
					for(register UInt32 i = 0 ; i < workers.Size() ; i++)
						if(workers[i] != null)
							delete workers[i];

					for(register UInt32 i = 0 ; i < processingElements.Size() ; i++)
						if(processingElements[i] != null)
							delete processingElements[i];

					if(controller != null)
						delete controller;

					if(mutex_completion != null)
						delete mutex_completion;
					if(waitcv_completion != null)
						delete waitcv_completion;

					if(mutex_simulation_phases != null)
						delete mutex_simulation_phases;
					if(waitcv_simulation_phases != null)
						delete waitcv_simulation_phases;

					for(register UInt32 i = 0 ; i < objects->Size(); i++)
					{
						LogicalProcess::TimeWarpInfo *twi = (*objects)[i]->twi;
						if(twi != null)
						{
							delete twi;
							(*objects)[i]->twi = null;
						}
					}
				}

				if(objects != null)
					delete objects;

				disposed = true;
			}
		}
		//----------------------------------------------------
		void TimeWarpSimulationManager::RegisterLogicalProcess(LogicalProcess *lp)
		{
			if(disposed)
				throw ObjectDisposedException("ConservativeSimulationManager", "The ConservativeSimulationManager has been disposed");

			if(lp == null)
				throw ArgumentNullException("lp", "lp is null");

			objects->PushBack(lp);
			lp->SetSimulationManager(this);
		}
		//----------------------------------------------------
		void TimeWarpSimulationManager::SendEvent(EventInterface *e)
		{
			//if(disposed)
			//	throw ObjectDisposedException("ConservativeSimulationManager", "The ConservativeSimulationManager has been disposed");
			//if(e == null)
			//	throw ArgumentNullException("e", "e is null");
			//Int32 srcPeID = e->GetSenderReference()->GetOwnerClusterID();
			//Int32 destPeID = e->GetReceiverReference()->GetOwnerClusterID();
			//ProcessingElement *destPe = processingElements[destPeID];
			ProcessingElement *destPe = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference())->twi->ownerPe;
			e->SetEventType(PositiveMessage);

			// ***** Save a negative message corresponding to the e into outputQueue ***** //
			EventInterface *negE = e->Clone();
			negE->SetEventType(AntiMessage);
			LogicalProcess::GetLogicalProcessById(e->GetSenderReference())->twi->outputQueue->Add(negE, e->GetSendTime());
			//processingElements[srcPeID]->GetOutputQueue()->Add(negE, e->GetSendTime());
			// ************************************************************************** //
			// Enqueues the event e into the destination FIFO input channel.
			/*tw_mutex->Lock();
			cout << "SendEvent()";
			cout << " sender: " << e->GetSenderReference()->GetName().get_BaseStream() << " receiver: " << e->GetReceiverReference()->GetName().get_BaseStream();
			cout << " sendTime: " << e->GetSendTime() << " recvTime: " << e->GetReceiveTime();
			//cout << " srcPeID: " << srcPeID << " destPeID: " << destPeID << " src: " << e->GetSenderReference()->GetID() << " dest: " << e->GetReceiverReference()->GetID();
			cout << endl;
			tw_mutex->Unlock();*/

			destPe->EnqueueMessage(e);
		}
		//----------------------------------------------------
		EventInterface *TimeWarpSimulationManager::GetNextEvent(LogicalProcess *lp)
		{
			ProcessingElement *destPe = processingElements[lp->GetOwnerClusterID()];

			return destPe->dequeuedMessage;
		}
		//----------------------------------------------------
		UInt64 TimeWarpSimulationManager::GetNextEventTime(LogicalProcess *lp)
		{
			return -1;
		}
		//----------------------------------------------------
		EventInterface *TimeWarpSimulationManager::PeekNextEvent(LogicalProcess *lp)
		{
			return null;
		}
		//----------------------------------------------------
		UInt32 TimeWarpSimulationManager::GetNumberOfLogicalProcesses()
		{
			//if(disposed)
			//	throw ObjectDisposedException("TimeWarpSimulationManager", "The TimeWarpSimulationManager has been disposed");

			return objects->Size();
		}
		//----------------------------------------------------
		void TimeWarpSimulationManager::Run(UInt64 simulateUntil)
		{
			if(disposed)
				throw ObjectDisposedException("TimeWarpSimulationManager", "The TimeWarpSimulationManager has been disposed");

			if(simulateUntil <= 0)
				throw ArgumentOutOfRangeException("simulateUntil", "simulateUntil is less than or equal to zero.");

			if(GetNumberOfLogicalProcesses() == 0)
				throw ArgumentOutOfRangeException("GetNumberOfLogicalProcesses", "You must register at least one logical process instance");

			this->simulateUntil = simulateUntil;

			if(!started)
			{
				started = true;
				cout << "\n\t xSim v2.2.1 (Many-Core Release)\n\t Optimistic Protocol with Time Warp\n";
				cout << "\t Copyright (c) 1999-" << DateTime::get_Now().get_Year() << " by Shahed University,\n";
				cout << "\t ALL RIGHTS RESERVED\n\n" << endl;
				cout << "Initializing parallel simulation environment ..." << endl;

				PrepareForParallelSimulation();
			}
			else
				throw ThreadStateException("The logical processor has already been started");
		}
		//----------------------------------------------------
		void TimeWarpSimulationManager::WaitOnManagerCompletion()
		{
			if(disposed)
				throw ObjectDisposedException("ConservativeSimulationManager", "The ConservativeSimulationManager has been disposed");

			if(!started)
				throw InvalidOperationException("Run() method must first be called");

			mutex_completion->Lock();
			while( completedThreadCount != workers.Size())
				waitcv_completion->Wait();
			mutex_completion->Unlock();
			cout << "Finalizing simulation ...\n";

			UInt64 numberOfProcessedEvents = 0;
			for(register UInt32 i = 0 ; i < processingElements.Size() ; i++)
				numberOfProcessedEvents += processingElements[i]->GetNumberOfProcessedEvents();
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
				<< density << " events/sec" << ").\n";
			cout << endl;
			/*density = 0;
			if(totalSimulationTime != 0)
				density = numberOfDeadlocks / totalSimulationTime;

			cout << numberOfDeadlocks << " deadlocks, " << density << " deadlocks/sec executed on " << clusters.Size() << " logical processors with " << Environment::get_ProcessorCount() << " physical cores).\n" << endl;
			//#if defined __Parvicursor_xSim_Debug_Enable__

			for(register Int32 i = 0 ; i < clusters.Size() ; i++)
				cout << "Number of blocking states in Cluster " << i << " is " << clusters[i]->blockingCount << ".\n";
			cout << endl;
			//#endif
			usage u;
			ResourceProfiler::GetResourceUsage(&u);
			ResourceProfiler::PrintResourceUsage(&u);*/

			Dispose(); ///
			return ;
		}
		//----------------------------------------------------
		LogicalProcess *TimeWarpSimulationManager::GetLogicalProcessById(UInt32 id)
		{
			if(id < 0)
				throw ArgumentOutOfRangeException("id", "id is less than zero");

			return (*logicalProcesses)[id];
		}
		//----------------------------------------------------
		UInt64 TimeWarpSimulationManager::GetSimulationTime()
		{
			if(disposed)
				throw ObjectDisposedException("TimeWarpSimulationManager", "The TimeWarpSimulationManager has been disposed");

			// Here, we must consider a lock-based or lock-free implementation for this method.

			return simulationTime;
		}
		//----------------------------------------------------
		void TimeWarpSimulationManager::SetSimulationTime(UInt64 newTime)
		{
			if(disposed)
				throw ObjectDisposedException("TimeWarpSimulationManager", "The TimeWarpSimulationManager has been disposed");

			// Here, we must consider a lock-based or lock-free implementation for this method.
			simulationTime = newTime;
		}
		//----------------------------------------------------
		void TimeWarpSimulationManager::PrepareForParallelSimulation()
		{
			if(disposed)
				throw ObjectDisposedException("TimeWarpSimulationManager", "The TimeWarpSimulationManager has been disposed");

			if(hasPreparedForParallelSimulation)
				return ;

			UInt32 originalNumberOfCores = numOfLogicalProcessors;
			UInt32 numberOfCores = originalNumberOfCores;

			if(originalNumberOfCores <= 1)
			{
				numberOfCores = 2;
				originalNumberOfCores = 2;
			}

			if(objects->Size() < numberOfCores)
				numberOfCores = objects->Size();

			// ***************************** Partitioning **************************** //
			partitions = partitioner->Partition(objects, numberOfCores);

			if(partitions == null)
				throw ArgumentNullException("Partitioner.Partition()", "Partitioner.Partition() returned a null value");

			///if(numberOfCores > originalNumberOfCores)
			///	throw ArgumentOutOfRangeException("numberOfPartitions", "The updated numberOfPartitions is greater than the requested value by the simulation kernel");

			threadCounter = LockFreeCounter();
			tw_global_termination_counter = LockFreeCounter(numberOfCores);

			for(register UInt32 i = 0 ; i < numberOfCores ; i++)
				processingElements.PushBack(new ProcessingElement(i));

			for(register UInt32 currentPeID = 0 ; currentPeID < partitions->GetNumberOfPartitions() ; currentPeID++)
			{
				Vector<LogicalProcess *> *partition = partitions->GetPartitionByIndex(currentPeID);
				ProcessingElement *ownerPe = processingElements[currentPeID];
				for(register UInt32 lpIndex = 0 ; lpIndex < partition->Size() ; lpIndex++)
				{
					// Source
					LogicalProcess *lp = (*partition)[lpIndex];
					lp->SetOwnerClusterID(currentPeID);
					lp->twi = new LogicalProcess::TimeWarpInfo(ownerPe, ProcessingElement::OnDisposeProcessedOrOutputQueue, ProcessingElement::OnRemoveNegQueue);
				}
			}

			if(on_partitioning_completion_callback != null)
				on_partitioning_completion_callback(on_partitioning_completion_state);
			// ***************************** Thread Creation ************************* //
			cout << "Running ..." << endl;

			mutex_completion = new Mutex();
			waitcv_completion = new ConditionVariable(mutex_completion);

			mutex_simulation_phases = new Mutex();
			waitcv_simulation_phases = new ConditionVariable(mutex_simulation_phases);

			controller = new Thread(Wrapper_To_Call_Controller, (void *)this);
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
		void TimeWarpSimulationManager::Worker()
		{
			// Initialization
			//Int32 ClusterID = (Int32)threadCounter.get_Count_And_Increment();
			Long PeID = threadCounter.get_Count_And_Increment();
			ProcessingElement *pe = processingElements[PeID];
			Vector<LogicalProcess *> *partiotion= partitions->GetPartitionByIndex(PeID);
			cout << "I'm Worker Thread " << PeID << endl;
			//

			// ******************* Performs TimeWarp Simulation ******************** //
			for(register UInt32 i = 0 ; i < partiotion->Size() ; i++)
				(*partiotion)[i]->Elaborate();

			for(register UInt32 i = 0 ; i < partiotion->Size() ; i++)
			{
				LogicalProcess *lp = (*partiotion)[i];
				lp->Initialize();
				lp->GetState()->SetOwnerLogicalProcess(lp);
			}

			// **** Begin Barrier Phase **** //
			SimulationPhasesBarrier();
			// **** End Barrier Phase ****** //

			if(PeID == processingElements.Size() - 1)
				xParvicursor_gettimeofday(&start, null);

			EventInterface *nextEvent = null;
			EventInterface *nextMessage = null;
			UInt64 receiveTimeNextEvent;
			UInt64 receiveTimeNextMessage;
			LogicalProcess *receiver = null;
			Int32 canBlock = false;
			const Int32 batch = 1;

			while(true/*pe->localVirtualTime < simulateUntil*/)
			{
				if(pe->unprocessedQueue->GetCount() == 0)
					canBlock = true;
				else
					canBlock = false;

				pe->inputChannel->DequeueAll(ProcessingElement::OnDequeueInputChannel, canBlock);

				if(pe->inputChannelState == ProcessingElement::DeadlockState)
				{
					cout << "deadlock state in PE " << PeID << endl;
					break;
				}

				/*nextMessage = pe->inputChannel->Dequeue(canBlock);
				// The inputChannel contains at least one message
				if(canBlock != -1)
				{
					if(nextMessage->GetEventType() == TerminationHasRequestedMessage)
					{
						delete (InternalEvent *)nextEvent;
						break;
					}

					receiveTimeNextMessage = nextMessage->GetReceiveTime();
					pe->unprocessedQueue->Enqueue(nextMessage, receiveTimeNextMessage);
				}*/

				//for(register UInt32 i = 0 ; i < batch ; i++)
				for(register UInt32 i = batch - 1 ; i != -1 ; i--) // A faster loop.
				{
					if(pe->unprocessedQueue->GetCount() != 0)
					{
						//nextEvent = pe->unprocessedQueue->Peek(receiveTimeNextEvent);
						//cout << "Peek PeId " << PeID << " unprocessedQueue->GetCount() " << pe->unprocessedQueue->GetCount() << " nextEvent " << nextEvent << " receiveTimeNextEvent " << receiveTimeNextEvent << endl;
						nextEvent = pe->unprocessedQueue->Dequeue(receiveTimeNextEvent);
						//cout << "Dequeue PeId " << PeID << " unprocessedQueue->GetCount() " << pe->unprocessedQueue->GetCount() << " nextEvent " << nextEvent << " receiveTimeNextEvent " << receiveTimeNextEvent << endl;
						//break;
						/*if(nextEvent == null)
						{
                            cout << "Dequeue PeId " << PeID << " unprocessedQueue->GetCount() " << pe->unprocessedQueue->GetCount()  << " nextEvent " << nextEvent << " receiveTimeNextEvent " << receiveTimeNextEvent << endl;
                            exit(0);
						}*/
						//cout << nextEvent << endl;
						receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
						receiveTimeNextEvent = nextEvent->GetReceiveTime();//
						// check for rollback.
						/*if(receiveTimeNextEvent < receiver->twi->localVirtualTime)
						{
							cout << "Rollback point was occurred on receiving a positive message in PE " << PeID;
							cout << " @lvt " << receiver->twi->localVirtualTime;
							cout << " receiveTime " << receiveTimeNextEvent;
							cout << " receive " << receiver->GetName().get_BaseStream();
							cout << endl;
							exit(0);
						}*/
						pe->dequeuedMessage = nextEvent;
						receiver->twi->localVirtualTime = receiveTimeNextEvent;
						receiver->SetSimulationTime(receiveTimeNextEvent);
						State *newState = nextEvent->GetState();
						State *curState = receiver->GetState();
						if(newState == null)
						{
							newState = receiver->AllocateState();
							nextEvent->SetState(newState);
						}
						newState->CopyState(curState);
						receiver->twi->processedQueue->Add(nextEvent, receiveTimeNextEvent);
						receiver->ExecuteProcess();
						pe->SetNumberOfProcessedEvents( pe->GetNumberOfProcessedEvents() + 1 );/// for state
						//if(pe->localVirtualTime < receiveTimeNextEvent)
							//pe->localVirtualTime = receiveTimeNextEvent;
					}
					else
						break;
				}

				/*tw_mutex->Lock();
				cout << PeID << " " << i << endl;
				tw_mutex->Unlock();
				i++;*/

				//if(pe->localInputChannel->GetCount() == 0)
				/*if(pe->unprocessedQueue->GetCount() == 0)
				{
					pe->inputChannel->DequeueAll(ProcessingElement::OnDequeueInputChannel);

					if(pe->inputChannelState == ProcessingElement::DeadlockState)
					{
						cout << "deadlock state in PE " << PeID << endl;
						break;
					}
				}
				else
				{
					while(pe->unprocessedQueue->GetCount() != 0 && pe->localVirtualTime < simulateUntil)
					{
						nextEvent = pe->unprocessedQueue->Dequeue(receiveTime);
						receiveTime = nextEvent->GetReceiveTime();
						LogicalProcess *receiver = nextEvent->GetReceiverReference();
						pe->localVirtualTime = receiveTime;
						receiver->SetSimulationTime(pe->localVirtualTime);
						State *s = nextEvent->GetState();
						if(s == null)
						{
							s = receiver->AllocateState();
							nextEvent->SetState(s);
						}
						s->CopyState(receiver->GetState());
						pe->dequeuedMessage = nextEvent;
						receiver->twi->processedQueue->Add(nextEvent, receiveTime);
						receiver->ExecuteProcess();
						pe->SetNumberOfProcessedEvents( pe->GetNumberOfProcessedEvents() + 1 );/// for state
					}

				}*/
				/*else
				{
					while(pe->localInputChannel->GetCount() != 0 && pe->localVirtualTime < simulateUntil)
					{
						nextEvent = pe->localInputChannel->Dequeue();
						receiveTime = nextEvent->GetReceiveTime();
						LogicalProcess *receiver = nextEvent->GetReceiverReference();
						pe->localVirtualTime = receiveTime;
						receiver->SetSimulationTime(pe->localVirtualTime);
						pe->dequeuedMessage = nextEvent;
						receiver->ExecuteProcess();
						pe->SetNumberOfProcessedEvents( pe->GetNumberOfProcessedEvents() + 1 );/// for state
					}

				}*/
				/*nextEvent = pe->DequeueMessage();

				if(nextEvent == null)
					break;
				else if(nextEvent->GetEventType() == TerminationHasRequestedMessage)
				{
					delete nextEvent;
					break;
				}

				receiveTime = nextEvent->GetReceiveTime();
				LogicalProcess *receiver = nextEvent->GetReceiverReference();/*/
				/************************************************************************/
				/*                      Check for rollback                              */
				/************************************************************************/
				/*if(receiveTime < pe->localVirtualTime) // Rollback occurred. Perform rollback here.
				{
					tw_mutex->Lock();
					cout << "Rollback occurred in PE " << PeID << " at localVirtualTime " << pe->localVirtualTime;
					cout << " sender: " << nextEvent->GetSenderReference()->GetName().get_BaseStream() << " receiver: " << nextEvent->GetReceiverReference()->GetName().get_BaseStream();
					cout << " sendTime: "  << nextEvent->GetSendTime() << " revcTime: " << nextEvent->GetReceiveTime();
					cout << endl;
					tw_mutex->Unlock();
					exit(0);
				}
				else // No rollback occurred*/
					//pe->localVirtualTime = receiveTime;
				/************************************************************************/
				/*                      State Saving                                    */
				// Save the LP state before executing the event
				// Consider here for periodic state saving. if pe->GetStateSavingPeriod(),
				// this means frequent state saving in traditional time warp proposed by
				// Jefferson.
				/************************************************************************/
				/*Int32 periodicStateSavingCounter = pe->GetStateSavingCounter();
				periodicStateSavingCounter++;
				if( periodicStateSavingCounter == pe->GetStateSavingPeriod() )
				{
					State *oldState = receiver->GetState();
					State *newState = receiver->AllocateState();
					newState->SetOwnerLogicalProcess(receiver);
					newState->CopyState(oldState);
					pe->GetStateQueue()->Add(newState, pe->localVirtualTime);
					pe->SetPeriodicStateSavingCounter(0);
				}
				else
					pe->SetPeriodicStateSavingCounter(periodicStateSavingCounter);*/

				/*tw_mutex->Lock();
				cout << "PE: " << PeID << " localVirtualTime: " << pe->localVirtualTime;
				cout << " sender: " << nextEvent->GetSenderReference()->GetName().get_BaseStream() << " receiver: " << nextEvent->GetReceiverReference()->GetName().get_BaseStream();
				cout << " sendTime: "  << nextEvent->GetSendTime() << " revcTime: " << nextEvent->GetReceiveTime();
				cout << endl;
				tw_mutex->Unlock();*/
				// ******************************************************** //
				/*receiver->SetSimulationTime(pe->localVirtualTime);
				receiver->ExecuteProcess();
				pe->SetNumberOfProcessedEvents( pe->GetNumberOfProcessedEvents() + 1 );/// for state
				*/
			}

			Long count_ = tw_global_termination_counter.Decrement_And_get_Count();
			if(count_ == 0)
			{
				tw_global_mutex_controller->Lock();
				tw_global_met = true;
				controllerThreadTerminationRequested = true;
				tw_global_waitcv_controller->Signal();
				tw_global_mutex_controller->Unlock();
			}

			printf("---------EndWhile-------- in PE %d count: %d localVirtualTime: %d \n", PeID, (Int32)tw_global_termination_counter.get_Count(), (Int32)pe->localVirtualTime); ///


			// **** Begin Barrier Phase **** //
			SimulationPhasesBarrier();
			// **** End Barrier Phase ****** //

			if(PeID == processingElements.Size() - 1)
				xParvicursor_gettimeofday(&stop, null);

			for(register Int32 i = 0 ; i < partiotion->Size() ; i++)
				(*partiotion)[i]->Finalize();


			// Worker Termination
			mutex_completion->Lock();
			completedThreadCount++;
			waitcv_completion->Signal();
			mutex_completion->Unlock();
		}
		//----------------------------------------------------
		void *TimeWarpSimulationManager::Wrapper_To_Call_Worker(void *pt2Object)
		{
			// explicitly cast to a pointer to TimeWarpSimulationManager
			TimeWarpSimulationManager *mySelf = (TimeWarpSimulationManager *)pt2Object;

			// call member
			mySelf->Worker();

			return pt2Object;
		}
		//----------------------------------------------------
		void TimeWarpSimulationManager::Controller()
		{
			cout << "I'm Controller" << endl;

			tw_global_mutex_controller->Lock();
			{
				while(!tw_global_met && !controllerThreadTerminationRequested)
					tw_global_waitcv_controller->Wait();
				if(controllerThreadTerminationRequested)
				{
					tw_global_mutex_controller->Unlock();
					goto EndWhile;
				}
				tw_global_met = false;
			}
			tw_global_mutex_controller->Unlock();

	EndWhile:

			// Global termination was detected.
			for(register UInt32 i = 0 ; i < processingElements.Size() ; i++)
			{
				ProcessingElement *pe = processingElements[i];
				InternalEvent *e = new InternalEvent(null, pe);
				e->SetEventType(TerminationHasRequestedMessage);
				pe->EnqueueMessage(e);
			}

			return ;
		}
		//----------------------------------------------------
		void *TimeWarpSimulationManager::Wrapper_To_Call_Controller(void *pt2Object)
		{
			// explicitly cast to a pointer to TimeWarpSimulationManager
			TimeWarpSimulationManager *mySelf = (TimeWarpSimulationManager *)pt2Object;

			// call member
			mySelf->Controller();

			return pt2Object;
		}
		//----------------------------------------------------
		void TimeWarpSimulationManager::SimulationPhasesBarrier()
		{
			mutex_simulation_phases->Lock();
			{
				completedThreadCount_simulation_phases++;
				if(completedThreadCount_simulation_phases == processingElements.Size())
				{
					completedThreadCount_simulation_phases = 0;
					waitcv_simulation_phases->Broadcast();
				}
				else
					waitcv_simulation_phases->Wait();
			}
			mutex_simulation_phases->Unlock();
		}
		//----------------------------------------------------
		void TimeWarpSimulationManager::RegisterOnPartitioningCompletionCallback(OnPartitioningCompletion callback, Object *state)
		{ 
			on_partitioning_completion_callback = callback; 
			on_partitioning_completion_state = state;
		}
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

