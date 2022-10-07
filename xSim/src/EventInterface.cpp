/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "EventInterface.h"
#include "LogicalProcess.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
        void EventInterface::Print(Int32 line)
        {
            std::cout << "Line " << line << " sender(" << GetSenderReference() << "," << LogicalProcess::GetLogicalProcessById(GetSenderReference())->GetName().get_BaseStream() << ")";
            std::cout << " receiver(" << GetReceiverReference() << "," << LogicalProcess::GetLogicalProcessById(GetReceiverReference())->GetName().get_BaseStream() << ")";
            std::cout << " sendTime " << GetSendTime();
            std::cout << " receiveTime " << GetReceiveTime() << std::endl;
        }
        //----------------------------------------------------
        /*PdesTime EventInterface::GetReceiveTimeLc()
        {
            //std::cout << "EventInterface::GetReceiveTimeLc() sender " << sender << std::endl;
            if(eventType == NormalMessage)
               //return PdesTime(receiveTime, lc, LogicalProcess::GetLogicalProcessById(sender)->processSpaceStateId); /// Attention for DSM execution.
               return PdesTime(receiveTime, lc, sender); /// Attention for DSM execution.
                //return PdesTime(receiveTime, lc, lc_cluster); /// Attention for DSM execution.
                //return PdesTime(receiveTime, lc, receiver); /// Attention for DSM execution.
                //return PdesTime(receiveTime, lc, LogicalProcess::GetLogicalProcessById(receiver)->processSpaceStateId); /// Attention for DSM execution.
            else
                return PdesTime(receiveTime, lc, sender);
        }*/
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
