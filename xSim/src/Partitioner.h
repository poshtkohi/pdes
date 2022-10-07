/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_Partitioner_h__
#define __Parvicursor_xSim_Partitioner_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>

#include "PartitionInfo.h"
#include "Vector.h"

using namespace System;
using namespace Parvicursor::xSim::psc;

//#include <vector>
//using namespace std;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		// Forward declarations
		//class LogicalProcess;
		enum PartitionerType { RoundRobin, TopologicalSort, Levelized, DepthFirstSearch, Metis, StronglyConnectedComponent, Customized };

		interface class Partitioner : public Object
		{
			/*---------------------fields-----------------*/
			/*---------------------methods----------------*/
			// Partition the objects.
			// This method is first invoked with the numberOfPartitions equal to the number of available physical processors
			// by xSim simulation manager. If the application needs fewer processors, then it can update numberOfPartitions due
			// to its passing by reference. If the number of modified numberOfPartitions is greater than
			// its original value passed by the simulation kernel, the kernel will throw an exception reporting this situation.
			// Caller is responsible for deleting both the arguments and the returned PartitionInfo.
			public: virtual PartitionInfo *Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions) = 0;
			/*--------------------------------------------*/

		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

