/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __Parvicursor_xSim_Samples_CircleLogicalProcess_h__
#define __Parvicursor_xSim_Samples_MonitorLogicalProcess_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/Random/Random.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>

#include <Parvicursor/xSim/DefaultEvent.h>
#include <Parvicursor/xSim/LogicalProcess.h>

using namespace System;
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
            class CircleLogicalProcess : public LogicalProcess
            {
                /*---------------------fields-----------------*/
                private: String objName;
                private: ofstream *logFile;
                private: bool disposed;
                /*---------------------methods----------------*/
                /// CircleLogicalProcess Class constructor.
                public: CircleLogicalProcess(const String &objName);
                /// CircleLogicalProcess Class destructor.
                public: ~CircleLogicalProcess();

                /// Overridden methods
				public: void Initialize();
				public: void Finalize();
                public: void ExecuteProcess();
                public: String GetName();

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

