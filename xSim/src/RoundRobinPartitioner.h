/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_RoundRobinPartitioner_h__
#define __Parvicursor_xSim_RoundRobinPartitioner_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>

#include "Partitioner.h"
#include "LogicalProcess.h"

using namespace System;

//#include <vector>
//using namespace std;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------

		// Forward declarations

		// class LogicalProcess;

		class RoundRobinPartitioner : public Partitioner
		{
			/*---------------------fields-----------------*/
			private: bool disposed;
			/*---------------------methods----------------*/
			// RoundRobinPartitioner Class constructor.
			public: RoundRobinPartitioner();
			// RoundRobinPartitioner Class destructor.
			public: ~RoundRobinPartitioner();

			/// Overridden methods
			public: PartitionInfo *Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions);
			/*--------------------------------------------*/

		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

