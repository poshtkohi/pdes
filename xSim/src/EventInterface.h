/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/


#ifndef __Parvicursor_xSim_EventInterface_h__
#define __Parvicursor_xSim_EventInterface_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>

#include "Globals.h"
#include "PdesTime.h"

using namespace System;

using namespace Parvicursor::PS2;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        //----------------------------------------------------
		// forward declarations
		class LogicalProcess;
		class SequentialSimulationManager;
		class DeadlockAvoidanceSimulationManager;
		class DeadlockDetectionSimulationManager;
		class TimeWarpSimulationManager;
		class ProcessingElement;
		class State;

		enum EventType
		{
			NormalMessage,
			NullMessage,
			AppNullMessage,
			RequestToRemoveTheChannelMessage,
			RecoveryHasRequestedMessage,
			TerminationHasRequestedMessage,
			TerminationHasRequestedMessageByUser,
			PositiveMessage,
			AntiMessage
		};

        interface class EventInterface : public Object
        {
			friend class SequentialSimulationManager;
			friend class DeadlockAvoidanceSimulationManager;
			friend class DeadlockDetectionSimulationManager;
			friend class TimeWarpSimulationManager;
			friend class ProcessingElement;
            /*---------------------fields-----------------*/
			//private: UInt64 loopCounter;
            private: UInt32 sender;
            private: UInt32 receiver;
            private: UInt64 sendTime;
            private: UInt64 receiveTime;
            private: UInt64 lc;
            //private: UInt32 lc_cluster;
            private: EventType eventType;
			public: bool deletable;///
			private: State *state;
			//private: UInt64 parent_id;
			//private: UInt64 child_id;
#if defined __space_time_graph_enabled__
			private: UInt32 id;
#endif
			//private: Int32 ownerChannel;
            /*---------------------methods----------------*/
			public: inline EventInterface()
			{
                deletable = true;///
				//ownerChannel = -2;

				//loopCounter = 0;
				eventType = NormalMessage;
				//sendTime = receiveTime = 0;
				//lc = 0;
				//state = null;
/*#if defined __Delta_Enabled__
				deltaCounter = 0;
#endif*/
			}
				//: eventType(NormalMessage), loopCounter(0) {};
            // Returns the send time of this event.
            public: inline UInt64 GetSendTime() { return sendTime; }
            // Gets time at which the event must be received and scheduled for execution.
            public: inline UInt64 GetReceiveTime() { return receiveTime; }
            //public: PdesTime GetReceiveTimeLc();
            public: inline PdesTime GetReceiveTimeLc()
            {
                return PdesTime(receiveTime, lc);
                //std::cout << "EventInterface::GetReceiveTimeLc() sender " << sender << std::endl;
                /*if(eventType == NormalMessage)
                   //return PdesTime(receiveTime, lc, LogicalProcess::GetLogicalProcessById(sender)->processSpaceStateId); /// Attention for DSM execution.
                   //return PdesTime(receiveTime, lc, sender); /// Attention for DSM execution.
                   //return PdesTime(receiveTime, lc, sender); /// Attention for DSM execution.
                    //return PdesTime(receiveTime, lc, lc_cluster); /// Attention for DSM execution.
                    //return PdesTime(receiveTime, lc, receiver); /// Attention for DSM execution.
                    //return PdesTime(receiveTime, lc, LogicalProcess::GetLogicalProcessById(receiver)->processSpaceStateId); /// Attention for DSM execution.
                else
                    return PdesTime(receiveTime, lc, sender);*/
            }
            // Returns the source (sender) of an event.
            public: inline UInt32 GetSenderReference() { return sender; }
            // Returns the destination (receiver) of an event.
            public: inline UInt32 GetReceiverReference() { return receiver; }
            public: inline void SetSendTime(UInt64 val) { sendTime = val; }
            public: inline void SetReceiveTime(UInt64 val) { receiveTime = val; }
            public: inline void SetSenderReference(UInt32 lp) { sender = lp; }
            public: inline void SetReceiverReference(UInt32 lp) { receiver = lp; }
			public: inline void SetEventType(EventType newType) { eventType = newType; }
			public: inline EventType GetEventType() { return eventType; }
			//----------------------------------------------------
			// Clones the event without copying eventType
			// This method is internally used by TimeWarpSimulationManager.
			private: inline EventInterface *Clone()
			{
				EventInterface *newE = Allocate();
				newE->sender = sender;
				newE->receiver = receiver;
				newE->sendTime = sendTime;
				newE->receiveTime = receiveTime;
				return newE;
			}
			//----------------------------------------------------
			// This method is internally used by TimeWarpSimulationManager.
			private: inline static EventInterface *Allocate()
			{
				EventInterface *newE = new EventInterface();
				return newE;
			}
			//----------------------------------------------------
			// This method is internally used by TimeWarpSimulationManager.
			private: inline static void Deaalocate(EventInterface *e)
			{
				delete e;
			}
			//----------------------------------------------------
			// This method is internally used by TimeWarpSimulationManager.
			private: inline static bool Compare(EventInterface &e1, EventInterface &e2)
			{
				if(e1.GetSenderReference() == e2.GetReceiverReference() && e1.GetReceiveTime() == e2.GetSendTime() && e1.GetSendTime() == e2.GetReceiveTime() )
				   return true;
				else
				//e1->GetReceiveTime();
					return false;
			}
			//----------------------------------------------------
			// This method is internally used by TimeWarpSimulationManager.
			private: inline bool operator==(EventInterface &e)
			{
				//this->GetReceiveTime();
				return Compare(*this, e);
			}
			//----------------------------------------------------
			// This method is internally used by TimeWarpSimulationManager.
			private : inline bool operator==(EventInterface *e)
			{
				//this->GetReceiveTime();
				return Compare(*this, *e);
			}
			//----------------------------------------------------
			// This method is internally used by TimeWarpSimulationManager.
			private: inline State *GetState()
			{
				return state;
			}
			//----------------------------------------------------
			// This method is internally used by TimeWarpSimulationManager.
			private: inline void SetState(State *s)
			{
				state = s;
			}
            //----------------------------------------------------
            private: void Print(Int32 line);
			//----------------------------------------------------
/*#if defined __Delta_Enabled__
			public: inline UInt64 GetDeltaCounter() { return deltaCounter; }
			public: inline void SetDeltaCounter(UInt64 newVal) { deltaCounter = newVal; }
#endif*/
			// Returns the destination (receiver) of an event.
			//public: virtual Int32 GetReceiverChannelIndex() = 0;
            /// Consider the operators new and delete o be re-implemented with supporting a memory pool for events.
            /// Overload operator new.
            //void* operator new(size_t) = 0;
            /// Overload operator delete.
            // void operator delete(void*) = 0;
            /*--------------------------------------------*/
        };
        //----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

