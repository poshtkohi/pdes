/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __Parvicursor_xSim_Samples_SinkLogicalProcess_h__
#define __Parvicursor_xSim_Samples_SinkLogicalProcess_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>
#include <System.IO/IOException/IOException.h>

#include <Parvicursor/xSim/LogicalProcess.h>

#include "QueueEvent.h"
#include "SinkState.h"

using namespace System;
using namespace System::IO;
using namespace Parvicursor::xSim;

#include <iostream>
#include <sstream>
#include <fstream>
using std::string;
using namespace std;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            class SinkLogicalProcess : public LogicalProcess
            {
                /*---------------------fields-----------------*/
                private: String objName;
                private: Int32 numEventProcessed;
				private: SinkState *myState;
//#ifdef __Parvicursor_xSim_Debug_Enable__
                private: ofstream *logFile;
//#endif
                private: bool disposed;
                /*---------------------methods----------------*/
                /// SinkLogicalProcess Class constructor.
                public: SinkLogicalProcess(const String &objName);
                /// SinkLogicalProcess Class destructor.
                public: ~SinkLogicalProcess();

				/// Overridden methods
				public: void Initialize();
				public: void Finalize();
				public: void ExecuteProcess();
				public: const String &GetName();
				public: State *GetState();
				public: State *AllocateState();
				public: void DeallocateState(const State *state);
				public: void DeallocateEvent(const EventInterface *e);

                /// Other methods
				public: void PrintInformation();
                /*--------------------------------------------*/
            };
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif

