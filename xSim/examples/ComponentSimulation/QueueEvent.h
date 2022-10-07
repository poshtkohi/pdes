/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __Parvicursor_xSim_Samples_QueueEvent_h__
#define __Parvicursor_xSim_Samples_QueueEvent_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>

#include <Parvicursor/xSim/EventInterface.h>
#include <Parvicursor/xSim/LogicalProcess.h>

using namespace System;
using namespace Parvicursor::xSim;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            class QueueEvent : public EventInterface
            {
                /*---------------------fields-----------------*/
                /*private: Int64 sendTime;
                private: Int64 receiveTime;
                private: LogicalProcess *sender;
                private: LogicalProcess *receiver;*/
                public: Int64 counter;
                private: bool disposed;
                /*---------------------methods----------------*/
                /// QueueEvent Class constructor.
                public: QueueEvent(Int64 sendTime, Int64 receiveTime, LogicalProcess *sender, LogicalProcess *receiver);
                /// QueueEvent Class destructor.
                public: ~QueueEvent();

                /// Overridden methods
                /*public: Int64 GetSendTime();
                public: Int64 GetReceiveTime();
                public: LogicalProcess *GetSenderReference();
                public: LogicalProcess *GetReceiverReference();*/

                /// Other methods

                /*--------------------------------------------*/

            };
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif

