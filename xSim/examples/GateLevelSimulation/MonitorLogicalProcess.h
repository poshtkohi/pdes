/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_Samples_MonitorLogicalProcess_h__
#define __Parvicursor_xSim_Samples_MonitorLogicalProcess_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>
#include <System.IO/IOException/IOException.h>

#include <Parvicursor/xSim/LogicalProcess.h>

#include "LogicEvent.h"

using namespace System;
using namespace System::IO;
using namespace Parvicursor::xSim;

#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            class MonitorLogicalProcess : public LogicalProcess
            {
                /*---------------------fields-----------------*/
                private: System::String objName;
                public: Int32 *inputBits;
                public: Int32 frame;
				public: Int32 numberOfInputs;
                private: ofstream *logFile;
                private: bool disposed;
                /*---------------------methods----------------*/
                /// MonitorLogicalProcess Class constructor.
                public: MonitorLogicalProcess(const System::String &objName, Int32 numberOfInputs);
                /// MonitorLogicalProcess Class destructor.
                public: ~MonitorLogicalProcess();

				/// Overridden methods
				public: void Initialize();
				public: void Finalize();
				public: void ExecuteProcess();
				public: const System::String &GetName();
				public: State *GetState();
				public: State *AllocateState();
				public: void DeallocateState(const State *state);
				public: void DeallocateEvent(const EventInterface *e);

                /// Other methods
                public: void ShowResults();
                public: void PrintInformation();
                public: Int32 GetObjectDelay();
                private: inline void SendNullMessages();
                /*--------------------------------------------*/
            };
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif


