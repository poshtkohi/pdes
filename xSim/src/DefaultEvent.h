/**
	#define meta ...
	printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/


#ifndef __Parvicursor_xSim_DefaultEvent_h__
#define __Parvicursor_xSim_DefaultEvent_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>

#include "EventInterface.h"
#include "LogicalProcess.h"

using namespace System;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		class DefaultEvent : public EventInterface
		{
			/*---------------------fields-----------------*/
			private: Object *data;
			/*---------------------methods----------------*/
			// DefaultEvent Class constructor.
			public: DefaultEvent(UInt64 sendTime, UInt64 receiveTime, UInt32 sender, UInt32 receiver, Object *data);
			// DefaultEvent Class destructor.
			public: ~DefaultEvent();

			/// Other methods
			// Gets the data associated with this event.
			public: Object *GetData();

			/*--------------------------------------------*/

		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

