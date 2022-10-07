/**
    #define meta ...
    printf("%s\n", meta);
**/

//---------------------------------------
#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/String/String.h>
#include <System/Convert/Convert.h>

#include <Parvicursor/xSim/SequentialSimulationManager.h>
#include <Parvicursor/xSim/ConservativeSimulationManager.h>
#include <Parvicursor/xSim/TimeWarpSimulationManager.h>
#include <Parvicursor/xSim/RoundRobinPartitioner.h>
#include <Parvicursor/xSim/TopologicalSortPartitioner.h>
#include <Parvicursor/xSim/SortedList.h>
#include <Parvicursor/xSim/BinaryHeap.h>
#include <Parvicursor/Profiler/ResourceProfiler.h>

#include "ServerLogicalProcess.h"
#include "SourceLogicalProcess.h"
//#include "SinkLogicalProcess.h"
//#include "PipelineTandemQueuePartitioner.h"

//_________________________________
//Parvicursor_RESOURCE_PROFILER_INITIALIZE()
//_________________________________

//---------------------------------------
using namespace System;

using namespace Parvicursor::Profiler;
using namespace Parvicursor::xSim;
using namespace Parvicursor::xSim::Samples;
//---------------------------------------
int main(int argc, char **argv)
{
	/**------------------Global Settings------------**/
	const Int32 numOfLogicalProcessors = 2;
	const Int64 simulateUntil = 100;
	const Int32 numOfServers = 100;//10000;
	const Int32 numOfPatterns = 100000;//1000;
	const Int32 channelCapacity = 128;
	const bool parallelExecution = false;
	/**------------------Source--------------------**/
	SourceLogicalProcess *source = new SourceLogicalProcess("source");
	ServerLogicalProcess *p1 = new ServerLogicalProcess("p1", P1P2);
	ServerLogicalProcess *p2 = new ServerLogicalProcess("p2", P1P2);
	ServerLogicalProcess *p3 = new ServerLogicalProcess("p3", P3);
	ServerLogicalProcess *p4 = new ServerLogicalProcess("p4", P4);

	source->AddOutputLP(p1->GetID());
	source->AddOutputLP(p4->GetID());
	source->AddOutputLP(p3->GetID());

	p1->AddOutputLP(p2->GetID());

	p2->AddOutputLP(p3->GetID());

	/**------------------Simulation----------------**/
	//RoundRobinPartitioner *partitioner = new RoundRobinPartitioner();
	TopologicalSortPartitioner *partitioner = new TopologicalSortPartitioner();
	SimulationManagerInterface *manager = null;

    if(parallelExecution)
		manager = new ConservativeSimulationManager(numOfLogicalProcessors, channelCapacity, partitioner);
	else
		manager = new SequentialSimulationManager(channelCapacity);

    manager->RegisterLogicalProcess(source);
	manager->RegisterLogicalProcess(p1);
	manager->RegisterLogicalProcess(p2);
	manager->RegisterLogicalProcess(p3);
	manager->RegisterLogicalProcess(p4);

	manager->Run(simulateUntil);
	manager->WaitOnManagerCompletion();
	
	/**------------------Termination---------------**/
	cout << "------------------------------------------------\n";
	delete partitioner;
	if(parallelExecution)
		delete (ConservativeSimulationManager *)manager;
	else
		delete (SequentialSimulationManager *)manager;

	// Logical processes must be deleted after deleting the simulation manager.
	delete source;
	delete p1;
	delete p2;
	delete p3;
	delete p4;

	cout << "The handles to all logical processes were successfully deleted.\nThe program is now terminating..." << endl;
	//getchar();
    return 0;
}
//---------------------------------------
