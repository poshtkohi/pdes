/**
	#define meta ...
	printf("%s\n", meta);
**/


#ifndef __Parvicursor_xSim_Samples_ClusteredQueuingNetworkPartitioner_h__
#define __Parvicursor_xSim_Samples_ClusteredQueuingNetworkPartitioner_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>

#include <Parvicursor/xSim/Partitioner.h>
#include <Parvicursor/xSim/LogicalProcess.h>

using namespace System;

using namespace Parvicursor::xSim;

//#include <vector>
//using namespace std;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		namespace Samples
		{

		//----------------------------------------------------

			// Forward declarations
			// class LogicalProcess;

			class ClusteredQueuingNetworkPartitioner : public Partitioner
			{
				/*---------------------fields-----------------*/
				private: Int32 numOfRows;
				private: Int32 numOfServersPerRow;
				private: bool disposed;
				/*---------------------methods----------------*/
				// ClusteredQueuingNetworkPartitioner Class constructor.
				public: ClusteredQueuingNetworkPartitioner(Int32 numOfRows, Int32 numOfServersPerRow);
				// ClusteredQueuingNetworkPartitioner Class destructor.
				public: ~ClusteredQueuingNetworkPartitioner();

				/// Overridden methods
				public: PartitionInfo *Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions);
				/*--------------------------------------------*/

			};
			//----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif

