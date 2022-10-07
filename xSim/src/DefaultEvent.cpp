/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "DefaultEvent.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		DefaultEvent::DefaultEvent(UInt64 sendTime, UInt64 receiveTime, UInt32 sender, UInt32 receiver, Object *data) : data(data)
		{
            SetSendTime(sendTime);
            SetReceiveTime(receiveTime);
            SetSenderReference(sender);
            SetReceiverReference(receiver);
		}
		//----------------------------------------------------
		DefaultEvent::~DefaultEvent()
		{
		}
		//----------------------------------------------------
		Object *DefaultEvent::GetData()
		{
			return data;
		}
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
