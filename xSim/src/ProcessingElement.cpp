/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "ProcessingElement.h"


//**************************************************************************************************************//
namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		ProcessingElement::ProcessingElement(Int32 PeID)
		{
			disposed = true;
			if(PeID < 0)
				throw ArgumentOutOfRangeException("PeID", "PeID is less than zero");

			peID = PeID;
			dequeuedMessage = null;
			//localInputChannel = new RingBuffer<EventInterface *>(128);
			inputChannel = new BlockingRingBuffer<EventInterface *>(128);
			//inputQueue = new SortedList();
			//stateQueue = new SortedList(OnRemoveStateQueue);
			//outputQueue = new SortedList(OnRemoveOutputQueue);
			unprocessedQueue = new BinaryHeap<EventInterface *>(128, OnRemoveUnprocessedQueue);
			numberOfProcessedEvents = 0;
			//stateSavingPeriod = 1;
			periodicStateSavingCounter = 0;
			localVirtualTime = 0;
			disposed = false;
		}
		//----------------------------------------------------
		ProcessingElement::~ProcessingElement()
		{
			//cout << "ProcessingElement::~ProcessingElement()" << endl;
			if(!disposed)
			{
				//delete localInputChannel;
				delete unprocessedQueue;
				delete inputChannel;
				//delete inputQueue;
				///delete stateQueue;
				///delete outputQueue;
				disposed = true;
			}
		}
		//----------------------------------------------------
		void ProcessingElement::EnqueueMessage(EventInterface *e)
		{
			// for e->GetSenderReference()->GetOwnerClusterID() == e->GetReceiverReference()->GetOwnerClusterID()
			// we must insert the event directly into unprocessedQueue.
			if(e->GetEventType() == TerminationHasRequestedMessage)
				inputChannel->Enqueue(e);
			else if(LogicalProcess::GetLogicalProcessById(e->GetSenderReference())->GetOwnerClusterID() == LogicalProcess::GetLogicalProcessById(e->GetReceiverReference())->GetOwnerClusterID())
				unprocessedQueue->Enqueue(e, e->GetReceiveTime());
				//localInputChannel->Enqueue(e);
			else
				inputChannel->Enqueue(e);

			//inputChannel->Enqueue(e);
		}
		//----------------------------------------------------
		/*EventInterface *ProcessingElement::DequeueMessage()
		{
			//dequeuedMessage = (EventInterface *)inputChannel->Dequeue();

			//dequeuedMessage = (EventInterface *)localInputChannel->Dequeue();

			//if(dequeuedMessage == null)
			dequeuedMessage = (EventInterface *)inputChannel->Dequeue();

			return dequeuedMessage;
		}*/
		//----------------------------------------------------
		void ProcessingElement::OnRemoveUnprocessedQueue(EventInterface *e)
		{
			// The antimessage annihilates the positive message
			// e is a positive message to be annihilated.
			State *s = e->GetState();
			LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference());
			if(s != null)
				receiver->DeallocateState(s);

			if(e->GetEventType() == PositiveMessage)
				receiver->DeallocateEvent(e);
			else
				EventInterface::Deaalocate(e);
			//cout << "ProcessingElement::OnRemoveUnprocessedQueue()" << endl;
		}
		//----------------------------------------------------
		void ProcessingElement::OnDisposeProcessedOrOutputQueue(EventInterface *e)
		{
			//cout << "ProcessingElement::OnDisposeProcessedOrOutputQueue() 1 GetEventType() " << e->GetEventType()  << endl;

			State *s = e->GetState();
			LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference());
			//cout << "ProcessingElement::OnDisposeProcessedOrOutputQueue() 1 GetEventType(): " << e->GetEventType()  << " receiver: " << receiver->GetName().get_BaseStream() << endl;
			if(s != null)
			{
				receiver->DeallocateState(s);
				e->SetState(null);
			}

			//cout << "ProcessingElement::OnDisposeProcessedOrOutputQueue() 2" << endl;

			if(e->GetEventType() == PositiveMessage)
				receiver->DeallocateEvent(e);
			else
				EventInterface::Deaalocate(e);
			//cout << "ProcessingElement::OnDisposeProcessedOrOutputQueue()" << endl;
		}
		//----------------------------------------------------
		void ProcessingElement::OnRemoveNegQueue(EventInterface *e)
		{
			EventInterface::Deaalocate(e);
			//cout << "ProcessingElement::OnRemoveNegQueue()" << endl;
		}
		//----------------------------------------------------
		// Backward
		/*void ProcessingElement::OnRemoveProcessedQueue(EventInterface *e)
		{
			LogicalProcess *receiver = e->GetReceiverReference();
			ProcessingElement *receiverPe = receiver->twi->ownerPe;
			// This means this is the first time that OnRemoveProcessedQueue is called.
			// The processedQueue is searched from the end to the start.
			if(receiver->twi->El == null)
			{
				receiver->twi->Ef = e;
				receiver->twi->El = e;
			}
			else
				receiver->twi->Ef = e;

			receiverPe->unprocessedQueue->Enqueue(e, e->GetReceiveTime());

			cout << "ProcessingElement::OnRemoveProcessedQueue()" << endl;
		}*/
		// Forward
		void ProcessingElement::OnRemoveProcessedQueue(EventInterface *e)
		{
			LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference());
			ProcessingElement *receiverPe = receiver->twi->ownerPe;
			// This means this is the first time that OnRemoveProcessedQueue is called.
			// The processedQueue is searched from the start to the end.
			if(receiver->twi->Ef == null)
			{
				receiver->twi->Ef = e;
				receiver->twi->El = e;
			}
			else
				receiver->twi->El = e;

			receiverPe->unprocessedQueue->Enqueue(e, e->GetReceiveTime());

			cout << "ProcessingElement::OnRemoveProcessedQueue()" << endl;
		}
		//----------------------------------------------------
		void ProcessingElement::OnRemoveProcessedQueueAntiMessageReceive(EventInterface *e)
		{
			LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference());
			ProcessingElement *receiverPe = receiver->twi->ownerPe;
			// This means this is the first time that OnRemoveProcessedQueue is called.
			// The processedQueue is searched from the start to the end.
			if(receiver->twi->Ef == null)
			{
				receiver->twi->Ef = e;
				receiver->twi->El = e;
				// We must annihilate Ef with the received antimessage, so we don't enqueue e into unprocessedQueue.
			}
			else
			{
				receiver->twi->El = e;
				receiverPe->unprocessedQueue->Enqueue(e, e->GetReceiveTime());
			}

			cout << "ProcessingElement::OnRemoveProcessedQueueAntiMessageReceive()" << endl;
		}
		//----------------------------------------------------
		void ProcessingElement::OnRemoveOutputQueue(EventInterface *e)
		{
			ProcessingElement *destPe = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference())->twi->ownerPe;
			destPe->EnqueueMessage(e);
			//cout << "ProcessingElement::OnRemoveOutputQueue()" << endl;
		}
		//----------------------------------------------------
		void ProcessingElement::OnDequeueInputChannel(EventInterface *e)
		{
			EventType eventType = e->GetEventType();

			if(eventType == TerminationHasRequestedMessage)
			{
				InternalEvent *ie = (InternalEvent *)e;
				ie->receiverPe->inputChannelState = DeadlockState;
				delete ie;
				return;
			}
			else
			{
				LogicalProcess *sender = LogicalProcess::GetLogicalProcessById(e->GetSenderReference());
				LogicalProcess *receiver = LogicalProcess::GetLogicalProcessById(e->GetReceiverReference());
				ProcessingElement *receiverPe = receiver->twi->ownerPe;
				UInt64 receiveTime = e->GetReceiveTime();

				if(eventType == PositiveMessage)
				{
					// If the message is in negQueue, we must destroy both of them.
					if(receiver->twi->negQueue->Remove(e))
					{
						State *s = e->GetState();
						if(s != null)
							receiver->DeallocateState(s);
						receiver->DeallocateEvent(e);
						return;
					}

					// Rollback detected.
					if(receiveTime < receiver->twi->localVirtualTime)
					{
						cout << "Rollback point was occurred on receiving a positive message in PE " << receiverPe->GetPeID();
						cout << " @lvt " << receiver->twi->localVirtualTime;
						cout << " receiveTime " << receiveTime;
						cout << " receive " << receiver->GetName().get_BaseStream();
						cout << endl;
						//exit(0);

						//------ Handling the Rollback ------------//
						receiver->twi->Ef = receiver->twi->El = null;
						// Locates Ef and El.
						receiver->twi->processedQueue->RemoveFromForward(receiveTime, OnRemoveProcessedQueue);
						cout << "Ef " << receiver->twi->Ef << " El " << receiver->twi->El << endl;
						// Restores the LP state to Tf.
						State *curState = receiver->GetState();
						State *newState = receiver->twi->Ef->GetState();
						curState->CopyState(newState);
						// Sends anti-messages in the interval [Tf, Tl] of the outputQueue
						receiver->twi->outputQueue->RemoveFromForward(receiver->twi->Ef->GetReceiveTime(), OnRemoveOutputQueue);
						//-----------------------------------------//
						receiverPe->unprocessedQueue->Enqueue(e, receiveTime);
					}
					else
						receiverPe->unprocessedQueue->Enqueue(e, receiveTime);
				}
				else if(eventType == AntiMessage)
				{
					/*cout << "AntiMessage event type was received in PE " << receiverPe->GetPeID();
					cout << " @lvt " << receiverPe->localVirtualTime;
					cout << " receiveTime " << receiveTime;
					cout << " EventType " << eventType;
					cout << endl;*/
					// If processesQueue contains a positive message corresponding to the antimessage, 
					// first rollback and then annihilate both of them.
					// Rollback detected.
					if(receiveTime < receiver->twi->localVirtualTime)
					{
						receiver->twi->Ef = receiver->twi->El = null;
						// Locates Ef and El, and performs rollback.
						if(receiver->twi->processedQueue->RemoveFromSearchForward(e, OnRemoveProcessedQueueAntiMessageReceive))
						{
							// Restores the LP state to Tf.
							State *curState = receiver->GetState();
							State *newState = receiver->twi->Ef->GetState();
							curState->CopyState(newState);
							// Sends anti-messages in the interval [Tf, Tl] of the outputQueue
							receiver->twi->outputQueue->RemoveFromForward(receiver->twi->Ef->GetReceiveTime(), OnRemoveOutputQueue);
							// Annihilates m+ and m-.
							State *s = receiver->twi->Ef->GetState();
							if(s != null)
								receiver->DeallocateState(s);
							receiver->DeallocateEvent(receiver->twi->Ef);

							s = e->GetState();
							if(s != null)
								receiver->DeallocateState(s);
							EventInterface::Deaalocate(e);

							return;

						}
					}
					// If unprocessesQueue contains a positive message corresponding to the antimessage, annihilate both of them.
					if(receiverPe->unprocessedQueue->Remove(e))
					{
						State *s = e->GetState();
						if(s != null)
							receiver->DeallocateState(s);
						EventInterface::Deaalocate(e);
						return;
					}
					else
						receiver->twi->negQueue->Add(e);
				}
				else
				{
					cout << "Unknown event type was received in PE " << receiverPe->GetPeID();
					cout << " @lvt " << receiverPe->localVirtualTime;
					cout << " receiveTime " << receiveTime;
					cout << " EventType " << eventType;
					cout << endl;
					exit(0);
				}
				/*
				receiverPe->inputChannelState = NormalState;
				receiverPe->localVirtualTime = receiveTime;
				receiver->SetSimulationTime(receiverPe->localVirtualTime);
				receiverPe->dequeuedMessage = e;
				receiver->ExecuteProcess();
				receiverPe->SetNumberOfProcessedEvents( receiverPe->GetNumberOfProcessedEvents() + 1 );/// for state*/
			}
			//cout << "ProcessingElement::OnDequeueInputChannel()" << endl;
		}
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
