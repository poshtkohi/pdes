/**
    #define meta ...
    printf("%s\n", meta);
**/

//---------------------------------------
#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/String/String.h>
#include <System/Convert/Convert.h>

#include <Parvicursor/xSim/SimulationManagerType.h>
#include <Parvicursor/xSim/SequentialSimulationManager.h>
#include <Parvicursor/xSim/DeadlockAvoidanceSimulationManager.h>
#include <Parvicursor/xSim/DeadlockDetectionSimulationManager.h>
#include <Parvicursor/xSim/TimeWarpSimulationManager.h>
#include <Parvicursor/xSim/RoundRobinPartitioner.h>
#include <Parvicursor/xSim/SortedList.h>
#include <Parvicursor/xSim/BinaryHeap.h>
#include <Parvicursor/Profiler/ResourceProfiler.h>

#include "ServerLogicalProcess.h"
#include "SourceLogicalProcess.h"
#include "SinkLogicalProcess.h"
#include "ClusteredQueuingNetworkPartitioner.h"

//_________________________________
//Parvicursor_RESOURCE_PROFILER_INITIALIZE()
//_________________________________

//---------------------------------------
using namespace System;

using namespace Parvicursor::Profiler;
using namespace Parvicursor::xSim;
using namespace Parvicursor::xSim::Samples;
using namespace Parvicursor::xSim::Collections;
//---------------------------------------
int main(int argc, char **argv)
{
#if defined WIN32 || WIN64
	// On Windows C runtime library (stdio, the one that provides fprintf and similar function) can open
	// no more than 512 files at once, this number can be increased up to 2048, but not further.
	// See _setmaxstdio. As a result, if fstream uses cstdio under the hood, same limit will apply to fstream.
	_setmaxstdio(2048);
#endif
	//Int32 x = 1024;
	//printf("nearestPower(%d): %d\n", x, nearestPower(x)); return 0;
	/**------------------Global Settings------------**/
    const Int32 numOfLogicalProcessors = 4;
    const Int32 simulateUntil = 100;
    const Int32 numOfRows = 8;//2;//16; //16
    const Int32 numOfServersPerRow = 10;//200;//4, 100
    const Int32 numOfPatterns = 10;//10; // 100
    const Int32 channelCapacity = 1;//4096;
	SimulationManagerType simulationManagerType;
	simulationManagerType = Sequential;
	//simulationManagerType = DeadlockAvoidance;
	simulationManagerType = DeadlockDetection;
	// Each server and object delay in this example is one.
	/**------------------Source--------------------**/
	String sourceObjectName = "source";
	SourceLogicalProcess *source = new SourceLogicalProcess(sourceObjectName, 1, numOfPatterns, numOfServersPerRow + 1);
	/**------------------Sink----------------------**/
	String sinkObjectName = "sink";
	SinkLogicalProcess *sink = new SinkLogicalProcess(sinkObjectName, numOfRows, numOfPatterns, numOfServersPerRow + 1);
	sink->AddOutputLP(source->GetID());
	/**------------------Rows----------------------**/
	vector<ServerLogicalProcess *> servers;
	// Creates servers in each row.
	for(register Int32 i = 0 ; i < numOfRows ; i++)
	{
		for(register Int32 j = 0 ; j < numOfServersPerRow ; j++)
		{
			String serverObjName = "server(" + Convert::ToString(i + 1) + ", " + Convert::ToString(j + 1) + ")";
			if(j == 0)
				servers.push_back(new ServerLogicalProcess(serverObjName, 2, 1/*i + j + 1*/, numOfServersPerRow, numOfPatterns));
			else
				servers.push_back(new ServerLogicalProcess(serverObjName, 1, 1/*i + j + 1*/, numOfServersPerRow, numOfPatterns));
			//cout << serverObjName.get_BaseStream() << endl;
		}
	}
	// Connects the servers together within each row.
	for(register Int32 i = 0 ; i < numOfRows ; i++)
	{
		for(register Int32 j = 0 ; j < numOfServersPerRow ; j++)
		{
			if(j == 0)
			{
				// Setups source output channels
				source->AddOutputLP(servers[i * numOfServersPerRow]->GetID());
			}

			if(j == numOfServersPerRow - 1)
			{
				// Setups servers[i * numOfServersPerRow + numOfServersPerRow - 1] output channels
				servers[i * numOfServersPerRow + j]->AddOutputLP(sink->GetID());
				servers[i * numOfServersPerRow + j]->AddOutputLP(servers[i * numOfServersPerRow]->GetID());
			}

			else
			{
				// Setups servers[i * numOfServersPerRow + j] output channels
				servers[i * numOfServersPerRow + j]->AddOutputLP(servers[i * numOfServersPerRow + j + 1]->GetID());
			}
		}
	}

	// Prints the information of all servers in each row.
#ifdef __Parvicursor_xSim_Debug_Enable__
	source->PrintInformation();
	sink->PrintInformation();
	for(register Int32 i = 0 ; i < numOfRows ; i++)
		for(register Int32 j = 0 ; j < numOfServersPerRow ; j++)
			servers[i * numOfServersPerRow + j]->PrintInformation();
#endif


	/**------------------Simulation----------------**/
	ClusteredQueuingNetworkPartitioner *partitioner = new ClusteredQueuingNetworkPartitioner(numOfRows, numOfServersPerRow);
	//RoundRobinPartitioner *partitioner = new RoundRobinPartitioner();
	SimulationManagerInterface *manager = null;

	switch(simulationManagerType)
	{
		case Sequential:
			manager = new SequentialSimulationManager(channelCapacity);
			break;
		case DeadlockAvoidance:
			manager = new DeadlockAvoidanceSimulationManager(numOfLogicalProcessors, channelCapacity, partitioner);
			break;
		case DeadlockDetection:
			manager = new DeadlockDetectionSimulationManager(numOfLogicalProcessors, channelCapacity, partitioner);
			break;
		default:
			manager = new SequentialSimulationManager(channelCapacity);
			break;
	}

	for(register Int32 i = 0 ; i < servers.size() ; i++)
		manager->RegisterLogicalProcess(servers[i]);

	manager->RegisterLogicalProcess(source);
	manager->RegisterLogicalProcess(sink);

	//_________________________________
	//Parvicursor_RESOURCE_PROFILER_BEGIN()
	//_________________________________

	manager->Run(simulateUntil);
	manager->WaitOnManagerCompletion();

	//_________________________________
	//Parvicursor_RESOURCE_PROFILER_END()
	//_________________________________

	//_________________________________
	//Parvicursor_RESOURCE_PROFILER_FINALIZE()
	//_________________________________


	/**------------------Termination---------------**/
	cout << "------------------------------------------------\n";
	delete source;
	delete sink;
	for(register Int32 i = 0 ; i < servers.size() ; i++)
		delete servers[i];
	delete partitioner;
	if(manager != null)
	{
		switch(simulationManagerType)
		{
			case Sequential:
				delete (SequentialSimulationManager *)manager;
				break;
			case DeadlockAvoidance:
				delete (DeadlockAvoidanceSimulationManager *)manager;
				break;
			case DeadlockDetection:
				delete (DeadlockDetectionSimulationManager *)manager;
				break;
			default:
				delete manager;
				break;
		}
	}

	cout << "The handles to all logical processes were successfully deleted.\nThe program is now terminating..." << endl;
    return 0;
}
//---------------------------------------
