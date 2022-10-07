/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_Samples_GateLogicalProcess_h__
#define __Parvicursor_xSim_Samples_GateLogicalProcess_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/Random/Random.h>
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
            enum GateTypes
            {
                AND_Type = 0,
                OR_Type = 1,
                INV_Type = 2,
                XOR_Type = 3,
				XNOR_Type = 4,
				NAND_Type = 5,
				NOR_Type = 6,
				BUF_Type = 7
            };
            //----------------------------------------------------
            class GateLogicalProcess : public LogicalProcess
            {
                /*---------------------fields-----------------*/
                private: GateTypes gateType;
                private: System::String objName;
                private: Int32 gateDelay;
                private: ofstream *logFile;
                private: bool disposed;
                // input bits
                public: Int32 *inputBits;
                public: Int32 frame;
                // output bit
                public: Int32 outputBit;
				public: Int32 lastOutputBit;
				private: Int32 *destPorts;
				public: Int32 numberOfInputs;
				public: Int32 numberOfOutputs;
				private: Double dummy1, dummy2, dummy3;
				private: Int32 counter;
                /*---------------------methods----------------*/
                /// GateLogicalProcess Class constructor.
                public: GateLogicalProcess(const System::String &objName, GateTypes gateType, Int32 numberOfInputs, Int32 numberOfOutputs, Int32 gateDelay, Int32 *destPorts);
                /// GateLogicalProcess Class destructor.
                public: ~GateLogicalProcess();

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
                private: inline Int32 ComputeOutput();
                private: inline void UpdateOutput();
                public: void PrintInformation();
                private: System::String GetGateType();
                //private: void SendTerminationEvent();
                public: Int32 GetObjectDelay();
                private: inline void SendNullMessages();
                private: inline void SendNullMessages1();
                /*--------------------------------------------*/

            };
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif


