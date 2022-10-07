/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_Samples_CircuitReader_h__
#define __Parvicursor_xSim_Samples_CircuitReader_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System.Collections/Queue/Queue.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>
#include <System/ArgumentException/ArgumentException.h>
#include <System/InvalidOperationException/InvalidOperationException.h>
#include <System.Threading/ThreadStateException/ThreadStateException.h>
#include <System.IO/IOException/IOException.h>

#include <Parvicursor/xSim/LogicalProcess.h>
#include <Parvicursor/xSim/SimulationManagerType.h>
#include <Parvicursor/xSim/SequentialSimulationManager.h>
#include <Parvicursor/xSim/DeadlockAvoidanceSimulationManager.h>
#include <Parvicursor/xSim/DeadlockDetectionSimulationManager.h>
#include <Parvicursor/xSim/Partitioner.h>
#include <Parvicursor/xSim/RoundRobinPartitioner.h>
#include <Parvicursor/xSim/DepthFirstSearchPartitioner.h>
#include <Parvicursor/xSim/TopologicalSortPartitioner.h>
#include <Parvicursor/xSim/LevelizedPartitioner.h>
//#include <Parvicursor/xSim/MetisPartitioner.h>


using namespace System;
using namespace System::IO;
using namespace System::Threading;
using namespace System::Collections;

using namespace Parvicursor::xSim;

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
using namespace std;

#include "DriverLogicalProcess.h"
#include "MonitorLogicalProcess.h"
#include "GateLogicalProcess.h"

/*#if !PatternsFromFile
#include <math.h>
#include <mpir.h>
#include <mpirxx.h>
#endif*/

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            class CircuitReader : public Object
            {
                private: enum LpType
                {
                    GATE = 0,
                    MONITOR = 1,
                    DRIVER = 2
                };
                private: struct Node
                {
                    LogicalProcess *lp;
                    Object *data1;
                    Object *data2;
                    LpType type;
                };
                /*---------------------fields-----------------*/
                private: ifstream *reader;
                private: std::map<std::string, Node *> lps;
                private: std::string driverLpName, monitorLpName;
                private: LogicalProcess *driverLp, *monitorLp;
                //private: Int32 criticalPathDelay;
                //private: Int32 lastCriticalPathDelay;
                //private: Queue *criticalPathDelays;
                private: Int32 inputChannelCapacity;
				private: SimulationManagerInterface *manager;
				private: SimulationManagerType simulationManagerType;
				private: Partitioner *partitioner;
				private: PartitionerType partitionerType;
				private: bool parallelExecution;
                private: bool elaborated;
                private: bool started;
                private: bool disposed;
                private: UInt32 _numOfLogicalProcessors;
                /*---------------------methods----------------*/
                /// CircuitReader Class constructor.
                public: CircuitReader(UInt32 numOfLogicalProcessors);
                /// CircuitReader Class destructor.
                public: ~CircuitReader();
                // Analyzes the input model file and constructs the netlist as a DAG (Directed Acyclic Graph) data structure.
                public: void Elaborate();
                public: void StartAll();
                public: void WaitOnAllCompletion();
                private: void HandleGateLP();
                private: void HandleMonitorLP();
                private: void HandleDriverLP();
                private: void DisposeLps();
                //private: Int32 FindCriticalPathDelay();
                //private: void DFS(HardwareLogicalProcess *v);
                /*--------------------------------------------*/
            };
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif

