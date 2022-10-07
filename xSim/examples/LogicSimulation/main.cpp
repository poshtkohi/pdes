/**
    #define meta ...
    printf("%s\n", meta);
**/

//---------------------------------------
#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/String/String.h>

#include <Parvicursor/xSim/SequentialSimulationManager.h>
#include <Parvicursor/xSim/ConservativeSimulationManager.h>
#include <Parvicursor/xSim/RoundRobinPartitioner.h>
#include <Parvicursor/Profiler/ResourceProfiler.h>

#include "GateLogicalProcess.h"
#include "DriverLogicalProcess.h"
#include "MonitorLogicalProcess.h"

//_________________________________
//Parvicursor_RESOURCE_PROFILER_INITIALIZE()
//_________________________________

//---------------------------------------
using namespace System;

using namespace Parvicursor::xSim;
using namespace Parvicursor::xSim::Samples;

//---------------------------------------
int main(int argc, char **argv)
{
	/**------------------Monitor--------------------**/
	String monitorObjectName = "monitor";
	Int32 numberOfMonitorInputs = 2;
	MonitorLogicalProcess *monitor = new MonitorLogicalProcess(monitorObjectName, numberOfMonitorInputs);
	monitor->PrintInformation();
	monitor->SetData((Object *)1);

	/**------------------Gates----------------------**/

	// and1
	String and1ObjectName = "and1";
	Int32 and1NumberOfInputs = 2;
	Int32 and1NumberOfOutputGates = 3;
	Int32 and1gateDelay = 1;
	Int32 *and1DestPortIDs = new Int32[and1NumberOfOutputGates];
	and1DestPortIDs[0] = 0;
	and1DestPortIDs[1] = 0;
	and1DestPortIDs[2] = 0;
	GateLogicalProcess *and1 = new GateLogicalProcess(and1ObjectName, AND_Type, and1NumberOfInputs, and1NumberOfOutputGates, and1gateDelay, and1DestPortIDs);
	and1->PrintInformation();
	and1->SetData((Object *)0);

	// or1
	String or1ObjectName = "or1";
	Int32 or1NumberOfInputs = 2;
	Int32 or1NumberOfOutputGates = 1;
	Int32 or1gateDelay = 1;
	Int32 *or1DestPortIDs = new Int32[or1NumberOfOutputGates];
	or1DestPortIDs[0] = 1;
	GateLogicalProcess *or1 = new GateLogicalProcess(or1ObjectName, OR_Type, or1NumberOfInputs, or1NumberOfOutputGates, or1gateDelay, or1DestPortIDs);
	or1->PrintInformation();
	or1->SetData((Object *)0);

	// or2
	String or2ObjectName = "or2";
	Int32 or2NumberOfInputs = 2;
	Int32 or2NumberOfOutputGates = 1;
	Int32 or2gateDelay = 1;
	Int32 *or2DestPortIDs = new Int32[or2NumberOfOutputGates];
	or2DestPortIDs[0] = 1;
	GateLogicalProcess *or2 = new GateLogicalProcess(or2ObjectName, OR_Type, or2NumberOfInputs, or2NumberOfOutputGates, or2gateDelay, or2DestPortIDs);
	or2->PrintInformation();
	or2->SetData((Object *)0);

	// Setups and1 output channels
	and1->AddOutputLP(monitor->GetID());
	and1->AddOutputLP(or2->GetID());
	and1->AddOutputLP(or1->GetID());

	// Setups or1 output channels
	or1->AddOutputLP(or2->GetID());

	// Setups or2 output channels
	or2->AddOutputLP(monitor->GetID());

	/**------------------Driver--------------------**/
	Int32 numberOfDriverOutputPorts = 3;
	OutputPort **outputPorts = new OutputPort *[numberOfDriverOutputPorts];
	String driverObjectName = "driver";
	Int32 criticalPathDelay = 3;

	// x0
	Int32 *port0DesPortIds = new Int32[1];
	port0DesPortIds[0] = 0;
	Int64 *port0destGateLogicalProcesses = new Int64[1];
	port0destGateLogicalProcesses[0] = and1->GetID();
	outputPorts[0] = new OutputPort();
	outputPorts[0]->bitValue = 0;
	outputPorts[0]->portId = 0;
	outputPorts[0]->numberOfOutputGates = 1;
	outputPorts[0]->destGateLogicalProcesses = port0destGateLogicalProcesses;
	outputPorts[0]->desPortIds = port0DesPortIds;

	// x1
	Int32 *port1DesPortIds = new Int32[1];
	port1DesPortIds[0] = 1;
	Int64 *port1destGateLogicalProcesses = new Int64[1];
	port1destGateLogicalProcesses[0] = and1->GetID();
	outputPorts[1] = new OutputPort();
	outputPorts[1]->bitValue = 0;
	outputPorts[1]->portId = 1;
	outputPorts[1]->numberOfOutputGates = 1;
	outputPorts[1]->destGateLogicalProcesses = port1destGateLogicalProcesses;
	outputPorts[1]->desPortIds = port1DesPortIds;

	// x2
	Int32 *port2DesPortIds = new Int32[1];
	port2DesPortIds[0] = 1;
	Int64 *port2destGateLogicalProcesses = new Int64[1];
	port2destGateLogicalProcesses[0] = or1->GetID();
	outputPorts[2] = new OutputPort();
	outputPorts[2]->bitValue = 0;
	outputPorts[2]->portId = 2;
	outputPorts[2]->numberOfOutputGates = 1;
	outputPorts[2]->destGateLogicalProcesses = port2destGateLogicalProcesses;
	outputPorts[2]->desPortIds = port2DesPortIds;

	DriverLogicalProcess *driver = new DriverLogicalProcess(driverObjectName, numberOfDriverOutputPorts, outputPorts, criticalPathDelay);
	driver->PrintInformation();

	// Setups driver output channels
	driver->AddOutputLP(and1->GetID());
	driver->AddOutputLP(and1->GetID());
	driver->AddOutputLP(or1->GetID());


	// Setups and1 input channels
	and1->AddInputLP(driver->GetID());
	and1->AddInputLP(driver->GetID());

	// Setups or1 input channels
	or1->AddInputLP(and1->GetID());
	or1->AddInputLP(driver->GetID());

	// Setups or2 input channels
	or2->AddInputLP(and1->GetID());
	or2->AddInputLP(or1->GetID());

	// Setups monitor input channels
	monitor->AddInputLP(and1->GetID());
	monitor->AddInputLP(or2->GetID());
	/**--------------------------------------------**/

	RoundRobinPartitioner *partitioner = new RoundRobinPartitioner();

//*
	ConservativeSimulationManager *manager = new ConservativeSimulationManager(1000, partitioner);
	manager->RegisterLogicalProcess(and1);
	manager->RegisterLogicalProcess(or1);
	manager->RegisterLogicalProcess(or2);
	manager->RegisterLogicalProcess(driver);
	manager->RegisterLogicalProcess(monitor);

	//_________________________________
	//Parvicursor_RESOURCE_PROFILER_BEGIN()
	//_________________________________

	manager->Run(10000);
	manager->WaitOnManagerCompletion();

	//_________________________________
	//Parvicursor_RESOURCE_PROFILER_END()
	//_________________________________

	//_________________________________
	//Parvicursor_RESOURCE_PROFILER_FINALIZE()
	//_________________________________
//*/


/*
	SequentialSimulationManager *manager = new SequentialSimulationManager();
	manager->RegisterLogicalProcess(and1);
	manager->RegisterLogicalProcess(or1);
	manager->RegisterLogicalProcess(or2);
	manager->RegisterLogicalProcess(driver);
	manager->RegisterLogicalProcess(monitor);

	//_________________________________
	//Parvicursor_RESOURCE_PROFILER_BEGIN()
	//_________________________________

	manager->Run(10000);
	manager->WaitOnManagerCompletion();

	//_________________________________
	//Parvicursor_RESOURCE_PROFILER_END()
	//_________________________________

	//_________________________________
	//Parvicursor_RESOURCE_PROFILER_FINALIZE()
	//_________________________________
*/


	cout << "------------------------------------------------\n";
	delete monitor;
	delete and1DestPortIDs;
	delete and1;
	delete or1DestPortIDs;
	delete or1;
	delete or2DestPortIDs;
	delete or2;
	for(Int32 i = 0 ; i < numberOfDriverOutputPorts ; i++)
		delete outputPorts[i];
	delete outputPorts;
	delete port0DesPortIds;
	delete port0destGateLogicalProcesses;
	delete port1DesPortIds;
	delete port1destGateLogicalProcesses;
	delete port2DesPortIds;
	delete port2destGateLogicalProcesses;
	delete driver;

	delete partitioner;
	delete manager;

	cout << "The handles to all logical processors were successfully deleted.\nThe program is now terminating..." << endl;

    return 0;
}
//---------------------------------------
