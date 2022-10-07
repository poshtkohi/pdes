/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/


#include "LogicEvent.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            LogicEvent::LogicEvent(Int64 sendTime, Int64 receiveTime, Int32 sender, Int32 receiver)
            {
				SetSendTime(sendTime);
				SetReceiveTime(receiveTime);
				SetSenderReference(sender);
				SetReceiverReference(receiver);

                //disposed = false;
            }
            //----------------------------------------------------
            LogicEvent::~LogicEvent()
            {
                /*if(!disposed)
                {
                    disposed = true;
                }*/
            }
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
