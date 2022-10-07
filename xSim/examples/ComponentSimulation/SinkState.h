/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __Parvicursor_xSim_Samples_SinkState_h__
#define __Parvicursor_xSim_Samples_SinkState_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>

#include <Parvicursor/xSim/EventInterface.h>
#include <Parvicursor/xSim/LogicalProcess.h>
#include <Parvicursor/xSim/State.h>

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
            class SinkState : public State
            {
                /*---------------------fields-----------------*/
				public: Int32 numEventProcessed;
                /*---------------------methods----------------*/
                /// SinkState Class constructor.
                public: SinkState()
				{
					numEventProcessed = 0;
				}
                /// SinkState Class destructor.
				public: ~SinkState() {}

                /// Overridden methods
				public: void CopyState(const State *toCopy)
				{
					//const SinkState *state = dynamic_cast<const SinkState *>(toCopy);
					const SinkState *state = (const SinkState *)toCopy;
					numEventProcessed = state->numEventProcessed;
				}

                /// Other methods

                /*--------------------------------------------*/

            };
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif

