/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __Parvicursor_xSim_Samples_SourceState_h__
#define __Parvicursor_xSim_Samples_SourceState_h__

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
            class SourceState : public State
            {
                /*---------------------fields-----------------*/
				public: Int64 frame;
				public: Int32 numEventProcessed;
				public: Int32 count;
                /*---------------------methods----------------*/
                /// SourceState Class constructor.
                public: SourceState()
				{
					frame = 0;
					numEventProcessed = 0;
					count = 0;
				}
                /// SourceState Class destructor.
				public: ~SourceState() {}

                /// Overridden methods
				public: void CopyState(const State *toCopy)
				{
					//const SourceState *state = dynamic_cast<const SourceState *>(toCopy);
					const SourceState *state = (const SourceState *)toCopy;
					frame = state->frame;
					numEventProcessed = state->numEventProcessed;
					count = state->count;
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

