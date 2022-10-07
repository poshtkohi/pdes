/**
    #define meta ...
    printf("%s\n", meta);
**/


#include "LogicEvent.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            LogicEvent::LogicEvent(Int64 sendTime, Int64 receiveTime, LogicalProcess *sender, LogicalProcess *receiver)
                                : sendTime(sendTime), receiveTime(receiveTime), sender(sender), receiver(receiver)
            {
                disposed = false;
            }
            //----------------------------------------------------
            LogicEvent::~LogicEvent()
            {
                if(!disposed)
                {
                    disposed = true;
                }
            }
            //----------------------------------------------------
            Int64 LogicEvent::GetSendTime()
            {
                return sendTime;
            }
            //----------------------------------------------------
            Int64 LogicEvent::GetReceiveTime()
            {
                return receiveTime;
            }
            //----------------------------------------------------
            LogicalProcess *LogicEvent::GetSenderReference()
            {
                return sender;
            }
            //----------------------------------------------------
            LogicalProcess *LogicEvent::GetReceiverReference()
            {
                return receiver;
            }
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
