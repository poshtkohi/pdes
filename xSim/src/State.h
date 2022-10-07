/**
    #define meta ...
    printf("%s\n", meta);
**/


#ifndef __Parvicursor_xSim_State_h__
#define __Parvicursor_xSim_State_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>

using namespace System;


//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		// forward declarations
		class LogicalProcess;
		class TimeWarpSimulationManager;
		class ProcessingElement;

		interface class State : public Object
		{
			friend class TimeWarpSimulationManager;
			friend class ProcessingElement;
            /*---------------------fields-----------------*/
			private: LogicalProcess *owner;
			/*---------------------methods----------------*/
			// Makes a copy of a given state.
			public: inline virtual void CopyState(const State *) = 0;
			private: inline void SetOwnerLogicalProcess(LogicalProcess *owner) { this->owner = owner; }
			private: inline LogicalProcess *GetOwnerLogicalProcess() { return owner; }
		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

