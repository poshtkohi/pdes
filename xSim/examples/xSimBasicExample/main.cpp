/**
    #define meta ...
    printf("%s\n", meta);
**/

//---------------------------------------
#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/String/String.h>

#include "CircleLogicalProcess.h"

#include <Parvicursor/xSim/SequentialSimulationManager.h>

//---------------------------------------
using namespace System;

using namespace Parvicursor::xSim;
using namespace Parvicursor::xSim::Samples;
//---------------------------------------
int main(int argc, char **argv)
{
	CircleLogicalProcess *circle1 = new CircleLogicalProcess("driver");
	CircleLogicalProcess *circle2 = new CircleLogicalProcess("monitor");

	circle1->AddOutputLP(circle2->GetID());
	circle2->AddInputLP(circle1->GetID());

	circle1->PrintInformation();
	circle2->PrintInformation();

	SequentialSimulationManager *manager = new SequentialSimulationManager(2);
	manager->RegisterLogicalProcess(circle1);
	manager->RegisterLogicalProcess(circle2);

	manager->Run(10);
	manager->WaitOnManagerCompletion();

	delete circle1;
	delete circle2;
	delete manager;
    return 0;
}
//---------------------------------------
