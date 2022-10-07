/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/


#ifndef __Parvicursor_xSim_InternalEvent_h__
#define __Parvicursor_xSim_InternalEvent_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>

using namespace System;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        //----------------------------------------------------
		// forward declarations
		class TimeWarpSimulationManager;
		class ProcessingElement;

        class InternalEvent : public EventInterface
        {
			friend class TimeWarpSimulationManager;
			friend class ProcessingElement;
            /*---------------------fields-----------------*/
			private: ProcessingElement *senderPe;
			private: ProcessingElement *receiverPe;
            /*---------------------methods----------------*/
			public: InternalEvent(ProcessingElement *senderPe, ProcessingElement *receiverPe) : senderPe(senderPe), receiverPe(receiverPe)
			{
			}
			//----------------------------------------------------
			public: ~InternalEvent()
			{
			}
			//----------------------------------------------------
			//----------------------------------------------------
            /*--------------------------------------------*/
        };
        //----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

