/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "SequentialSimulationManager.h"


// Stores the number of allocated logical processes.
extern UInt32 processCounter;
// Stores the allocated logical processes.
// We use an array to store the references to increase the access performance.
extern Vector<LogicalProcess *> *logicalProcesses;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
        SequentialSimulationManager::SequentialSimulationManager(UInt32 channelCapacity)
        {
            if(channelCapacity <= 0)
				throw ArgumentOutOfRangeException("channelCapacity", "channelCapacity is less than or equal to zero");

            this->channelCapacity = channelCapacity;
			objects = new vector<LogicalProcess *>;
			//pq = new DeadlockAvoidancePriorityQueue(channelCapacity, false, OnDisposePriorityQueue, null);
			numberOfProcessedEvents = 0;
			started = false;
			SetSimulationTime(0);
			on_partitioning_completion_callback = null;
			on_partitioning_completion_state = null;
			pq = priq_new(this->channelCapacity);
			previous_event = null;
			disposed = false;
#if defined __space_time_graph_enabled__
			stg = null;
#endif
			//end_of_simulation_callback = null;
            //disposed = false;
        }
        //----------------------------------------------------
        SequentialSimulationManager::~SequentialSimulationManager()
        {
			Dispose();
        }
		//----------------------------------------------------
		void SequentialSimulationManager::Dispose()
		{
			if(!disposed)
			{
                if(previous_event != null)
				{
                    if(previous_event->deletable)
                    {
                        LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(previous_event->GetReceiverReference());
                        receiver->DeallocateEvent(previous_event);
                    }
                    previous_event = null;
				}

				delete objects;
				//delete pq;
				/*ps2_priority pri;
				EventInterface *e;
				while( (e = (EventInterface *)priq_pop(pq, &pri)) != null)
					LogicalProcess::GetLogicalProcessById(e->GetReceiverReference())->DeallocateEvent(e);*/
				priq_delete(pq);
				/*#if defined __Delta_Enabled__
				delete deltaQueue;
				#endif*/
#if defined __space_time_graph_enabled__
				if(stg != null)
                    delete stg;
#endif

				disposed = true;
			}
		}
		//----------------------------------------------------
		void SequentialSimulationManager::RegisterLogicalProcess(LogicalProcess *lp)
		{
			//if(disposed)
			//	throw ObjectDisposedException("SequentialSimulationManager", "The SequentialSimulationManager has been disposed");

			if(lp == null)
				throw ArgumentNullException("lp", "lp is null");

			objects->push_back(lp);
			lp->SetSimulationManager(this);
		}
		//----------------------------------------------------
		UInt32 SequentialSimulationManager::GetNumberOfLogicalProcesses()
		{
			//if(disposed)
			//	throw ObjectDisposedException("SequentialSimulationManager", "The SequentialSimulationManager has been disposed");

			return objects->size();
		}
        //----------------------------------------------------
		void SequentialSimulationManager::Run(UInt64 simulateUntil)
		{
			//if(disposed)
			//	throw ObjectDisposedException("SequentialSimulationManager", "The SequentialSimulationManager has been disposed");

			if(simulateUntil < 0)
				throw ArgumentOutOfRangeException("simulateUntil", "simulateUntil is less than zero.");

			if(GetNumberOfLogicalProcesses() == 0)
				throw ArgumentOutOfRangeException("GetNumberOfLogicalProcesses", "You must register at least one logical process instance");

			if(!started)
			{
				started = true;

				if(on_partitioning_completion_callback != null)
					on_partitioning_completion_callback(on_partitioning_completion_state);

                //GenerateLpDotGraph("lpGraph.dot"); ///

				std::cout << "\n\t PS2 v2.2.1 (Many-Core Release) - Sequential Protocol\n";
				std::cout << "\t Parvicursor Simulation System\n";
				std::cout << "\t Copyright (c) 2004-" << DateTime::get_Now().get_Year() << " by Shahed University,\n";
				std::cout << "\t ALL RIGHTS RESERVED\n\n";
				std::cout << "Initializing sequential simulation environment ...\n";
				std::cout << "Running ..." << std::endl;

#if defined __space_time_graph_enabled__
                stg = new SpaceTimeGraph();
                prepared = false;
                last_event_id = -1;
#endif

				for(register UInt32 i = 0 ; i < GetNumberOfLogicalProcesses() ; i++)
				{
					(*objects)[i]->SetOwnerClusterID(0);
					(*objects)[i]->Elaborate();
				}

				for(register UInt32 i = 0 ; i < GetNumberOfLogicalProcesses() ; i++)
				//for(register UInt32 i = GetNumberOfLogicalProcesses() - 1 ; i != -1 ; --i)
					(*objects)[i]->Initialize();
				//clock_t c1 = clock();

				//Vector<EventInterface *> deltaEvents = Vector<EventInterface *>(512);

				struct timeval start; // has tv_sec and tv_usec elements.
				xParvicursor_gettimeofday(&start, null);

                UInt64 timestamp = -1;
				EventInterface *nextEvent = null;
				EventInterface *curEvent = null;

				/*nextEvent = (EventInterface *)pq->PeekInternal(timestamp);
				if(nextEvent != null)
					SetSimulationTime(timestamp);

				// For timed and delta events
				while(nextEvent != null && GetSimulationTime() < simulateUntil)
				{
					// For delta events
					//Int32 save = 0;
					while(nextEvent != null && GetSimulationTime() == nextEvent->GetReceiveTime() )
					{
						nextEvent = (EventInterface *)pq->DequeueInternal(timestamp);
						LogicalProcess *receiver = nextEvent->GetReceiverReference();
						if(!receiver->marked)
						{
							deltaEvents.PushBack(nextEvent);
							receiver->marked = true;
						}
						else
						{
							cout << "hello0" << endl;
							if(nextEvent->deletable)
							{
								cout << "hello1" << endl;
								receiver->DeallocateEvent(nextEvent);
							}
						}

						nextEvent = (EventInterface *)pq->PeekInternal(timestamp) ;
					}

					//for(register Int32 i = 0 ; i < deltaEvents.Size() ; i++)
					for(register Int32 i = deltaEvents.Size() - 1 ; i != -1 ; --i) // fifo -> stack for pq
					{
						EventInterface *deltaEvent = deltaEvents[i];
						LogicalProcess *receiver = deltaEvent->GetReceiverReference();
						receiver->SetSimulationTime(GetSimulationTime());
						receiver->e = deltaEvent;
						receiver->ExecuteProcess();
						receiver->marked = false;
						if(deltaEvent->deletable)
							receiver->DeallocateEvent(deltaEvent);
					}

					deltaEvents.Reset();

					nextEvent = (EventInterface *)pq->PeekInternal(timestamp);
					if(nextEvent != null)
					{
						nextEvent = (EventInterface *)pq->DequeueInternal(timestamp);
						SetSimulationTime(timestamp);
						LogicalProcess *receiver = nextEvent->GetReceiverReference();
						receiver->SetSimulationTime(timestamp);
						//receiver->pq = pq;
						receiver->e = nextEvent;
						receiver->marked = false;
						//cout << "nextEvent->GetReceiveTime(): " << nextEvent->GetReceiveTime() << " GetSimulationTime: " << GetSimulationTime() << endl;
						receiver->ExecuteProcess();
						if(nextEvent->deletable)
							receiver->DeallocateEvent(nextEvent);
					}

					nextEvent = (EventInterface *)pq->PeekInternal(timestamp);
					SetSimulationTime(timestamp);
				}*/

				ps2_priority pri;

				nextEvent = (EventInterface *)priq_top(pq, &pri);

				while(nextEvent != null && GetSimulationTime() < simulateUntil)
				{
					///nextEvent = (EventInterface *)priq_pop(pq, &pri);
					LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
					//receiver->pq = pq;
					//receiver->marked = true;
					///receiver->e = nextEvent;
                    if(nextEvent->GetEventType() == TerminationHasRequestedMessageByUser)
                    {
                        //std::cout << "TerminationHasRequestedMessageByUser" << std::endl;
                        delete nextEvent;
                        break;
                    }
                    receiver->SetSimulationTime(pri.timestamp);
                    SetSimulationTime(pri.timestamp);
					receiver->ExecuteProcess();
					//receiver->marked = false;
					//curEvent = nextEvent;
					nextEvent = (EventInterface *)priq_top(pq, &pri);
                    //if(curEvent->deletable)
                    //    receiver->DeallocateEvent(curEvent);
				}

				/*nextEvent = (EventInterface *)priq_pop(pq, &pri);

				while(nextEvent != null && GetSimulationTime() < simulateUntil)
				{
					//nextEvent = (EventInterface *)priq_pop(pq, &pri);
					SetSimulationTime(pri.timestamp);
					LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(nextEvent->GetReceiverReference());
					receiver->SetSimulationTime(pri.timestamp);
					//receiver->pq = pq;
					//receiver->marked = true;
					receiver->e = nextEvent;
					receiver->ExecuteProcess();
					//receiver->marked = false;
					curEvent = nextEvent;
					//nextEvent = (EventInterface *)priq_top(pq, &pri);
					nextEvent = (EventInterface *)priq_pop(pq, &pri);
                    if(curEvent->deletable)
                        receiver->DeallocateEvent(curEvent);
				}*/

				//printf("---------EndWhile-------- in Cluster 0 localSimulationTime: %d ", (Int32)GetSimulationTime()); ///

				struct timeval stop; // has tv_sec and tv_usec elements.
				xParvicursor_gettimeofday(&stop, null);

				for(register UInt32 i = 0 ; i < GetNumberOfLogicalProcesses() ; i++)
					(*objects)[i]->Finalize();

				 std::cout << "Finalizing simulation ...\n";

				 Double d1, d2;
				 d1 = (Double)start.tv_sec + 1e-6*((Double)start.tv_usec);
				 d2 = (Double)stop.tv_sec + 1e-6*((Double)stop.tv_usec);
				 // return result in seconds
				 totalSimulationTime = d2 - d1;

				 UInt32 density = 0;
				 if(totalSimulationTime != 0)
					 density = numberOfProcessedEvents / totalSimulationTime;

				 std::cout << "Simulation completed (" << numberOfProcessedEvents << " events in "
					 << totalSimulationTime << " secs, "
					 << density << " events/sec executed on 1 logical processor).\n" << std::endl;

				 //cout << "c2 - c1: " << c2 - c1 << endl;

				 std::cout << "\n---------------- Runtime Statistics ----------------\n\n";
				 usage u;
				 ResourceProfiler::GetResourceUsage(&u);
				 ResourceProfiler::PrintResourceUsage(&u);

#if defined __space_time_graph_enabled__
                /*UInt32 event_counter = 0;
                EventInformation *send = null;
                EventInformation *sendLast = null;
                EventInformation *receive = null;
                EventInformation *receiveLast = null;
                std::cout << "sendList " << sendList.size() << " receiveList " << receiveList.size() << std::endl;
                for(std::unordered_map<Int64, std::vector<EventInformation *>* >::iterator it = sendList.begin() ; it != sendList.end() ; ++it)
                {
                    Int64 sendTime = it->first;
                    std::vector<EventInformation *> *sends = it->second;
                    send = null;
                    for(std::vector<EventInformation *>::iterator it1 = sends->begin() ; it1 != sends->end() ; ++it1)
                    {
                        sendLast = send;
                        send = *it1; /// e1
                        if(sendLast != null && send->sender == sendLast->sender)
                        {
                            send->id = sendLast->id;
                        }
                        else
                        {
                            if(send->id == -1)
                            {
                                //send->id = event_counter;
                                send->id = stg->vertices.size();
                                stg->AddVertex(send->sender, send->sendTime);
                            }
                            //event_counter++;
                        }
                        //std::cout << "sender " << send->sender << " receiver " << send->receiver << " sendTime " << send->sendTime << std::endl;
                        for(std::unordered_map<Int64, std::vector<EventInformation *>* >::iterator it2 = receiveList.begin() ; it2 != receiveList.end() ; ++it2)
                        {
                            Int64 recvTime = it2->first;
                            std::vector<EventInformation *> *receives = it2->second;
                            receive = null;
                            for(std::vector<EventInformation *>::iterator it3 = receives->begin() ; it3 != receives->end() ; ++it3)
                            {
                                receiveLast = receive;
                                receive = *it3;
                                if(*send == *receive) /// e2
                                {

                                    if(receive->id == -1)
                                    {
                                        //receive->id = event_counter;
                                        receive->id = stg->vertices.size();
                                        stg->AddVertex(receive->receiver, receive->recvTime);
                                        //event_counter++;
                                    }
                                    stg->AddEdge(send->id, receive->id);
                                }
                            }
                        }
                    }
                }


                for(std::unordered_map<Int64, std::vector<EventInformation *>* >::iterator it = sendList.begin() ; it != sendList.end() ; ++it)
                {
                    Int64 sendTime = it->first;
                    std::vector<EventInformation *> *sends = it->second;
                    for(std::vector<EventInformation *>::iterator it1 = sends->begin() ; it1 != sends->end() ; ++it1)
                    {
                        EventInformation *send = *it1;
                        if(send->id == -1)
                            std::cout << *send << std::endl;
                    }
                }

                for(std::unordered_map<Int64, std::vector<EventInformation *>* >::iterator it = receiveList.begin() ; it != receiveList.end() ; ++it)
                {
                    Int64 receiveTime = it->first;
                    std::vector<EventInformation *> *receives = it->second;
                    for(std::vector<EventInformation *>::iterator it1 = receives->begin() ; it1 != receives->end() ; ++it1)
                    {
                        EventInformation *receive = *it1;
                        if(receive->id == -1)
                            std::cout << *receive << std::endl;
                    }
                }*/
                std::cout << "Spacetime graph size " << stg->vertices.size() << std::endl;
                stg->GenerateDotGraph("stg.dot");
                stg->GeneratePriorities();
#endif

				 Dispose();

			}
			else
				throw ThreadStateException("The simulation has already been started");
		}
		//----------------------------------------------------
        void SequentialSimulationManager::SendEvent(EventInterface *e)
        {
            priq_push(pq, e, e->GetReceiveTime());

#if defined __space_time_graph_enabled__

            if(stg->vertices.size() == 0)
            {
                e->id = stg->vertices.size();
                stg->AddVertex(CreatedBySendEvent, e->GetSenderReference(), e->GetSenderReference(), e->GetReceiverReference(), e->GetSendTime());
                //last_event_id = e->id;
                //e->parent_id = event_counter;
                //last_event_id = e->parent_id;
                //e->id = event_counter;
                //event_counter++;
            }
            else
            {
                if(e->GetSenderReference() == stg->vertices[stg->vertices.size() - 1]->generator) // ? sendtime != receiveTime ?
                //if(e->GetSenderReference() == stg->vertices[stg->vertices.size() - 1]->sender)
                {
                    //stg->AddVertex(e->GetSenderReference(), e->GetReceiverReference(), e->GetReceiveTime());
                    //stg->AddEdge(e->id, event_counter);
                    //e->parent_id = stg->vertices[stg->vertices.size() - 1]->id;
                    e->id = stg->vertices[stg->vertices.size() - 1]->id;
                }
                else
                {
                    e->id = stg->vertices.size();
                    stg->AddVertex(CreatedBySendEvent, e->GetSenderReference(), e->GetSenderReference(), e->GetReceiverReference(), e->GetSendTime());

                    //if(last_event_time == e->GetSendTime())
                    if(last_event_id != -1)/// && e->GetSendTime() == e->GetReceiveTime()) // ?
                    {
                        stg->AddEdge(last_event_id, e->id);
                    }
                    //e->id = event_counter;
                    //event_counter++;
                    //e->parent_id = last_event_id;
                    //event_counter++;
                }
            }

            //e->parent_id = event_counter;
            //event_counter++;
            /*EventInformation *info = new EventInformation;
            info->sender = e->GetSenderReference();
            info->receiver = e->GetReceiverReference();
            info->sendTime = e->GetSendTime();
            info->recvTime = e->GetReceiveTime();
            info->counter = pq->counter;
            if(sendList.count(info->sendTime) == 0)
                sendList.insert(std::pair<Int64, std::vector<EventInformation *>* >(info->sendTime, new std::vector<EventInformation *>));
            sendList[info->sendTime]->push_back(info);*/
#endif
        }
        //----------------------------------------------------
        EventInterface *SequentialSimulationManager::GetNextEvent(LogicalProcess *lp)
        {
            if(previous_event != null)
            {
                if(previous_event->deletable)
                {
                    LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(previous_event->GetReceiverReference());
                    receiver->DeallocateEvent(previous_event);
                }
            }
            ps2_priority pri;
            EventInterface *e = (EventInterface *)priq_pop(pq, &pri);
            previous_event = e;
            //UInt64 timestamp;
            //EventInterface *e = (EventInterface *)pq->DequeueInternal(timestamp);

            if(e != null)
                numberOfProcessedEvents++;

#if defined __space_time_graph_enabled__
            //if(GetSimulationTime() < 1 || !prepared)
            //    return e;

            /*if(e->GetReceiverReference() == stg->vertices[stg->vertices.size() - 1]->receiver) // ? sendtime != receiveTime ?
            {
                if(e->GetSenderReference() == e->GetReceiverReference())
                {
                    stg->AddVertex(e->GetSenderReference(), e->GetReceiverReference(), e->GetReceiveTime());
                    stg->AddEdge(e->id, event_counter);
                    event_counter++;
                }
                else
                    stg->AddEdge(e->id, stg->vertices[stg->vertices.size() - 1]->id);
            }
            else
            {
                //std::cout << "event_counter " << event_counter << " vertices.size() " << stg->vertices.size() << std::endl;
                stg->AddVertex(e->GetSenderReference(), e->GetReceiverReference(), e->GetReceiveTime());
                stg->AddEdge(e->id, event_counter);
                event_counter++;
            }*/
            //if(e->parent_id == e->child_id)
            //    stg->AddVertex(e->GetSenderReference(), e->GetReceiverReference(), e->GetReceiveTime());
            //else
            //    stg->AddEdge(e->parent_id, e->child_id);

            if(e->GetReceiverReference() == stg->vertices[stg->vertices.size() - 1]->generator)// ? sendtime != receiveTime ?
            //if(e->GetReceiverReference() == stg->vertices[stg->vertices.size() - 1]->receiver)
            {
                //e->child_id = stg->vertices[stg->vertices.size() - 1]->id;
                //stg->AddEdge(e->parent_id, e->child_id);
                //last_event_id = e->child_id;
                if(last_event_id != e->id)
                {
                    stg->AddEdge(e->id, stg->vertices[stg->vertices.size() - 1]->id);
                    e->id = stg->vertices[stg->vertices.size() - 1]->id;
                }
                //last_event_id = e->id;
                //std::cout << "hello" << std::endl;
            }
            else
            {
                UInt32 id = e->id;
                e->id = stg->vertices.size();
                stg->AddVertex(CreatedByGetNextEvent, e->GetReceiverReference(), e->GetSenderReference(), e->GetReceiverReference(), e->GetReceiveTime());
                //e->child_id = event_counter;
                //stg->AddEdge(e->parent_id, e->child_id);
                stg->AddEdge(id, e->id);
                //e->id = event_counter;
                //event_counter++;
                //last_event_id = e->parent_id;
                last_event_id = e->id;
            }

            //last_event_id = e->id;

            //last_event_time = e->GetReceiveTime();

            //std::cout << "parent " << e->parent_id << " child " << e->child_id << std::endl;


            /*EventInformation *info = new EventInformation;
            info->sender = e->GetSenderReference();
            info->receiver = e->GetReceiverReference();
            info->sendTime = e->GetSendTime();
            info->recvTime = e->GetReceiveTime();
            info->counter = pri.delta;
            if(receiveList.count(info->recvTime) == 0)
                receiveList.insert(std::pair<Int64, std::vector<EventInformation *>* >(info->recvTime, new std::vector<EventInformation *>));
            receiveList[info->recvTime]->push_back(info);*/
#endif

            return e;
        }
        //----------------------------------------------------
        EventInterface *SequentialSimulationManager::PeekNextEvent(LogicalProcess *lp)
        {
            /*ps2_priority pri;
            EventInterface *e = (EventInterface *)priq_top(pq, &pri);
            if(e != null && e->GetEventType() == TerminationHasRequestedMessageByUser)
            {
                return null;
            }
            return e;*/

            ps2_priority pri;
            EventInterface *e = (EventInterface *)priq_top(pq, &pri);

            if(e != null)
            {
                if(e->GetEventType() == TerminationHasRequestedMessageByUser)
                    return null;
                else if(e->GetReceiveTime() == lp->GetSimulationTime())
                {
                    if(e->GetReceiverReference() == lp->GetID())
                        return e;
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
		void SequentialSimulationManager::WaitOnManagerCompletion()
		{
			//if(disposed)
			//	throw ObjectDisposedException("SequentialSimulationManager", "The SequentialSimulationManager has been disposed");

			if(!started)
				throw InvalidOperationException("Run() method must first be called");
		}
		//----------------------------------------------------
		LogicalProcess *SequentialSimulationManager::GetLogicalProcessById(UInt32 id)
		{
			//if(id < 0)
			//	throw ArgumentOutOfRangeException("id", "id is less than zero");

			return (*logicalProcesses)[id];
		}
		//----------------------------------------------------
		void SequentialSimulationManager::OnDisposePriorityQueue(EventInterface *e)
		{
			//return;
			if(e == null)
				return;

			LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference());
			//cout << "SequentialSimulationManager::OnDisposePriorityQueue receiver: " << receiver->GetID()  << endl;
			if(receiver != null)
			{
                if(e->deletable)
                    receiver->DeallocateEvent(e);
            }
		}
		//----------------------------------------------------
		/*void SequentialSimulationManager::RegisterEndOfSimulationCallback(EndOfSimulation callback)
		{
			if(callback == null)
				throw ArgumentNullException("callback", "callback is null");

			end_of_simulation_callback = callback;
		}*/
		//----------------------------------------------------
		/*UInt64 SequentialSimulationManager::GetSimulationTime()
		{
			if(disposed)
				throw ObjectDisposedException("SequentialSimulationManager", "The SequentialSimulationManager has been disposed");

			return simulationTime;
		}*/
		//----------------------------------------------------
		/*void SequentialSimulationManager::SetSimulationTime(UInt64 newTime)
		{
			if(disposed)
				throw ObjectDisposedException("SequentialSimulationManager", "The SequentialSimulationManager has been disposed");

			simulationTime = newTime;
		}*/
		//----------------------------------------------------
		void SequentialSimulationManager::RegisterOnPartitioningCompletionCallback(OnPartitioningCompletion callback, Object *state)
		{
			on_partitioning_completion_callback = callback;
			on_partitioning_completion_state = state;
		}
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
