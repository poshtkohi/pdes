/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __Parvicursor_xSim_Samples_GateLogicalProcess_h__
#define __Parvicursor_xSim_Samples_GateLogicalProcess_h__

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
#include "MonitorLogicalProcess.h"

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
            enum GateTypes
            {
                AND_Type = 0,
                OR_Type = 1,
                INV_Type = 2,
                XOR_Type = 3,
				XNOR_Type = 4,
				NAND_Type = 5,
				NOR_Type = 6
            };
            //----------------------------------------------------
            class GateLogicalProcess : public LogicalProcess
            {
                /*---------------------fields-----------------*/
                private: GateTypes gateType;
                private: String objName;
                private: Int32 gateDelay;
                private: ofstream *logFile;
                private: bool disposed;
                // input bits
                public: Int32 *inputBits;
                public: Int32 frame;
                // output bit
                public: Int32 outputBit;
				public: Int32 lastOutputBit;
                public: Int32 numInputValue;
                private: Int32 numEventProcessed;
				private: Int32 *destPorts;
				public: Int32 numberOfInputs;
				public: Int32 numberOfOutputs;
                /*---------------------methods----------------*/
                /// GateLogicalProcess Class constructor.
                public: GateLogicalProcess(const String &objName, GateTypes gateType, Int32 numberOfInputs, Int32 numberOfOutputs, Int32 gateDelay, Int32 *destPorts);
                /// GateLogicalProcess Class destructor.
                public: ~GateLogicalProcess();

                /// Overridden methods
				public: void Initialize();
				public: void Finalize();
				public: void ExecuteProcess();
				public: String GetName();

                /// Other methods
                public: Int32 ComputeOutput();
                public: void UpdateOutput();
                public: void PrintInformation();
                private: String GetGateType();
                //private: void SendTerminationEvent();
                public: Int32 GetObjectDelay();
                /*--------------------------------------------*/

            };
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif

