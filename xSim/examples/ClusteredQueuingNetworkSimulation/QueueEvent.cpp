/**
	#define meta ...
	printf("%s\n", meta);
**/


#include "QueueEvent.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		namespace Samples
		{
			//----------------------------------------------------
			QueueEvent::QueueEvent(UInt64 sendTime, UInt64 receiveTime, UInt32 sender, UInt32 receiver)
				//: sendTime(sendTime), receiveTime(receiveTime), sender(sender), receiver(receiver)
			{
				SetSendTime(sendTime);
				SetReceiveTime(receiveTime);
				SetSenderReference(sender);
				SetReceiverReference(receiver);
				//EventInterface();
				SetEventType(NormalMessage);
				disposed = false;
			}
			//----------------------------------------------------
			QueueEvent::~QueueEvent()
			{
				if(!disposed)
				{
					disposed = true;
				}
			}
			//----------------------------------------------------
			/*Int64 QueueEvent::GetSendTime()
			{
			return sendTime;
			}
			//----------------------------------------------------
			Int64 QueueEvent::GetReceiveTime()
			{
			return receiveTime;
			}
			//----------------------------------------------------
			LogicalProcess *QueueEvent::GetSenderReference()
			{
			return sender;
			}
			//----------------------------------------------------
			LogicalProcess *QueueEvent::GetReceiverReference()
			{
			return receiver;
			}*/
			//----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
