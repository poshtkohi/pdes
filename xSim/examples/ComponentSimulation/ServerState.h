/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __Parvicursor_xSim_Samples_ServerState_h__
#define __Parvicursor_xSim_Samples_ServerState_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>

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
            class ServerState : public State
            {
                /*---------------------fields-----------------*/
				public: Int32 numEventProcessed;
                /*---------------------methods----------------*/
                /// ServerState Class constructor.
                public: ServerState()
				{
					numEventProcessed = 0;
				}
                /// ServerState Class destructor.
				public: ~ServerState() {}

                /// Overridden methods
				public: void CopyState(const State *toCopy)
				{
					//const ServerState *state = dynamic_cast<const ServerState *>(toCopy);
					const ServerState *state = (const ServerState *)toCopy;
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

