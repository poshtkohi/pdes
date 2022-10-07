/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_PartitionInfo_h__
#define __Parvicursor_xSim_PartitionInfo_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>

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
		class LogicalProcess;

		class PartitionInfo : public Object
		{
			/*---------------------fields-----------------*/
			private: Int32 numberOfPartitions;
			/// Stores the partitions (clusters) of the parallel simulation.
			private: Vector<Vector<LogicalProcess *> *> partitions;
			private: bool disposed;
			/*---------------------methods----------------*/
			// PartitionInfo Class constructor.
			public: PartitionInfo();
			// PartitionInfo Class destructor.
			public: ~PartitionInfo();
			// Gets the number of partition (clusters) within this instance.
			public: Int32 GetNumberOfPartitions();
			// Gets the partition (cluster) specified by index.
			public: Vector<LogicalProcess *> *GetPartitionByIndex(Int32 index);
			// Adds a new partition (cluster) into PartitionInfo instance.
			public: void AddPartition(Vector<LogicalProcess *> *partition);
			// Prints the information of all partitions in the PartitionInfo instance.
			public: void PrintInformation();
			/*--------------------------------------------*/

		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

