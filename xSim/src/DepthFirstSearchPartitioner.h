/**
	#define meta ...
	printf("%s\n", meta);
**/


#ifndef __Parvicursor_xSim_DepthFirstSearchPartitioner_h__
#define __Parvicursor_xSim_DepthFirstSearchPartitioner_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/Random/Random.h>
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

		class DepthFirstSearchPartitioner : public Partitioner
		{
			/*---------------------fields-----------------*/
			private: bool disposed;
			private: LogicalProcess *root;
			private: UInt32 currentCore;
			private: UInt32 numberOfCores;
			private: UInt32 numberOfPaths;
			private: Vector<UInt32> cores;
			/*---------------------methods----------------*/
			// DepthFirstSearchPartitioner Class constructor.
			public: DepthFirstSearchPartitioner(LogicalProcess *root);
			// DepthFirstSearchPartitioner Class destructor.
			public: ~DepthFirstSearchPartitioner();

			/// Overridden methods
			public: PartitionInfo *Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions);
			private: void DFS(LogicalProcess *v, Vector< Vector<LogicalProcess*>* > *partitions);
			private: void DFS_ComputeNumberOfPaths(LogicalProcess *v);
			/*--------------------------------------------*/

		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

