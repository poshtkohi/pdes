/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_Samples_LogicEvent_h__
#define __Parvicursor_xSim_Samples_LogicEvent_h__

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
            class LogicEvent : public EventInterface
            {
                /*---------------------fields-----------------*/
                public: Int32 bitValue;
                // Source port ID
                public: Int32 sourcePort;
                // Destination port ID
                public: Int32 destinationPort;
                public: Int32 frame;
                /*---------------------methods----------------*/
                /// LogicEvent Class constructor.
                public: LogicEvent(Int64 sendTime, Int64 receiveTime, Int32 sender, Int32 receiver);
                /// LogicEvent Class destructor.
                public: ~LogicEvent();

                /// Overridden methods

                /// Other methods

                /*--------------------------------------------*/

            };
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif

