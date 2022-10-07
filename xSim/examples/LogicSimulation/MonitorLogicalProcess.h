/**
    #define meta ...
    printf("%s\n", meta);
**/

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
            class MonitorLogicalProcess : public LogicalProcess
            {
                /*---------------------fields-----------------*/
                private: String objName;
                public: Int32 *inputBits;
                public: Int32 frame;
                public: Int32 numInputValue;
                private: Int32 numEventProcessed;
				public: Int32 numberOfInputs;
                private: ofstream *logFile;
                private: bool disposed;
                /*---------------------methods----------------*/
                /// MonitorLogicalProcess Class constructor.
                public: MonitorLogicalProcess(const String &objName, Int32 numberOfInputs);
                /// MonitorLogicalProcess Class destructor.
                public: ~MonitorLogicalProcess();

				/// Overridden methods
				public: void Initialize();
				public: void Finalize();
				public: void ExecuteProcess();
				public: String GetName();

                /// Other methods
                public: void ShowResults();
                public: void PrintInformation();
                public: Int32 GetObjectDelay();
                /*--------------------------------------------*/
            };
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif

