/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "RoundRobinPartitioner.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		RoundRobinPartitioner::RoundRobinPartitioner()
		{
			disposed = false;
		}
		//----------------------------------------------------
		RoundRobinPartitioner::~RoundRobinPartitioner()
		{
			if(!disposed)
			{
				disposed = true;
			}
		}
		//----------------------------------------------------
		PartitionInfo *RoundRobinPartitioner::Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions)
		{
			if(disposed)
				throw ObjectDisposedException("RoundRobinPartitioner", "The RoundRobinPartitioner has been disposed");

			if(objects == null)
				throw ArgumentNullException("objects", "objects is null");

			if(numberOfPartitions <= 0)
				throw ArgumentOutOfRangeException("numberOfPartitions", "numberOfPartitions is less than or equal to zero");

			PartitionInfo *info = new PartitionInfo();

			Vector< Vector<LogicalProcess*>* > partitions(numberOfPartitions);

			for(register Int32 i = 0 ; i < numberOfPartitions ; i++)
				partitions[i] = new Vector<LogicalProcess *>;

			// Partition 0 for driver.
			/*(*objects)[0]->SetOwnerClusterID(0);
			partitions[0]->push_back((*objects)[0]);

			for(register Int32 i = 1, n = 1; i < objects->size() ; i++, n++)
			{
				if(n == numberOfPartitions)
					n = 1;
				(*objects)[i]->SetOwnerClusterID(n);
				partitions[n]->push_back((*objects)[i]);
			}*/

			for(register Int32 i = 0, n = 0; i < objects->Size() ; i++, n++)
			{
				if(n == numberOfPartitions)
					n = 0;
				(*objects)[i]->SetOwnerClusterID(n);
				partitions[n]->PushBack((*objects)[i]);
			}

			for(register Int32 i = 0 ; i < numberOfPartitions ; i++)
				info->AddPartition(partitions[i]);

//#if defined __Parvicursor_xSim_Debug_Enable__
			info->PrintInformation();
//#endif

			return info;
		}
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
