/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_Samples_DriverLogicalProcess_h__
#define __Parvicursor_xSim_Samples_DriverLogicalProcess_h__

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
#include <vector>
using namespace std;


//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
			//----------------------------------------------------
            struct OutputPort
            {
                Int32 numberOfOutputGates;
                Int32 bitValue;
				Int32 lastBitValue;
                Int32 portId;
                // Set of destination object handles
                Int64 *destGateLogicalProcesses;
				Int32 *desPortIds;
				OutputPort() : bitValue(0), lastBitValue(0)
				{}
            };
            //----------------------------------------------------
            class DriverLogicalProcess : public LogicalProcess
            {
                /*---------------------fields-----------------*/
                private: System::String objName;
                private: Int32 numberOfOutputPorts;
				private: OutputPort **outputPorts;
                private: Int32 criticalPathDelay;
                private: Int32 frame;
                private: ofstream *logFile;
                private: fstream *patternReader;
                private: bool endOfPatternsReached;
                private: bool disposed;
                private: Int32 count;
				private: Int32 sent;
				private: Int32 numEventProcessed;
                /*---------------------methods----------------*/
                /// DriverLogicalProcess Class constructor.
                public: DriverLogicalProcess(const System::String &objName, Int32 numberOfOutputPorts, OutputPort **outputPorts, Int32 criticalPathDelay);
                /// DriverLogicalProcess Class destructor.
                public: ~DriverLogicalProcess();

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
                private: inline Int32 GetBinaryValue(char c);
                private: inline void CopyPatternToOutputPorts();
                public: void PrintInformation();
                private: void SendTerminationEvent();
                public: OutputPort **GetOutputPorts(Int32 &size);
                public: Int32 GetObjectDelay();
                /*--------------------------------------------*/

            };
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif

