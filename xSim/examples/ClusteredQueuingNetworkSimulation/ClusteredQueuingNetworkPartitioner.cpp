/**
	#define meta ...
	printf("%s\n", meta);
**/

#include "ClusteredQueuingNetworkPartitioner.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		namespace Samples
		{
			//----------------------------------------------------
			ClusteredQueuingNetworkPartitioner::ClusteredQueuingNetworkPartitioner(Int32 numOfRows, Int32 numOfServersPerRow) : numOfRows(numOfRows), numOfServersPerRow(numOfServersPerRow)
			{
				disposed = false;
			}
			//----------------------------------------------------
			ClusteredQueuingNetworkPartitioner::~ClusteredQueuingNetworkPartitioner()
			{
				if(!disposed)
				{
					disposed = true;
				}
			}
			//----------------------------------------------------
			PartitionInfo *ClusteredQueuingNetworkPartitioner::Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions)
			{
				if(disposed)
					throw ObjectDisposedException("ClusteredQueuingNetworkPartitioner", "The ClusteredQueuingNetworkPartitioner has been disposed");

				if(objects == null)
					throw ArgumentNullException("objects", "objects is null");

				if(numberOfPartitions <= 0)
					throw ArgumentOutOfRangeException("numberOfPartitions", "numberOfPartitions is less than or equal to zero");

				Int32 numOfCores = numberOfPartitions;

				if(numOfRows < numberOfPartitions)
					numberOfPartitions = numOfRows;

				PartitionInfo *info = new PartitionInfo(numberOfPartitions);

				Vector< Vector<LogicalProcess*>* > partitions(numberOfPartitions);

				for(register Int32 i = 0 ; i < numberOfPartitions ; i++)
					partitions[i] = new Vector<LogicalProcess *>;

				// In this partitioning algorithm, we assume that the source and sink objects are in the end of the objects Vector.
				// In the algorithm for the clustered queuing network, we allocate possibly each queue network row to each CPU core.
				// If there is more than two cores, we allocate the source and sink objects into one dedicated core to maximize
				// the simulation concurrency. Otherwise, if there is only two cores then we allocate the source and sink objects
				// into last core.

				// Distributes rows into CPU cores.
				// servers[i][j]
				Int32 core = 0;
				for(register Int32 i = 0 ; i < numOfRows ; i++)
				{
					for(register Int32 j = 0 ; j < numOfServersPerRow ; j++)
					{
						(*objects)[i * numOfServersPerRow + j]->SetOwnerClusterID(core);
						partitions[core]->PushBack((*objects)[i * numOfServersPerRow + j]);
						//(*objects)[i][j]->SetOwnerClusterID(core);
						//partitions[core]->push_back((*objects)[i][j]);
					}
					core++;
					if(core == numberOfPartitions)
						core = 0;
				}

				// Dispatches source and sink objects into the last core.

				/// Source object.
				(*objects)[objects->Size() - 2]->SetOwnerClusterID(numberOfPartitions - 1);
				partitions[numberOfPartitions - 1]->PushBack((*objects)[objects->Size() - 2]);

				/// Sink object.
				(*objects)[objects->Size() - 1]->SetOwnerClusterID(numberOfPartitions - 1);
				partitions[numberOfPartitions - 1]->PushBack((*objects)[objects->Size() - 1]);


				for(register Int32 i = 0 ; i < numberOfPartitions ; i++)
					info->AddPartition(partitions[i]);

#ifdef __Parvicursor_xSim_Debug_Enable__
				info->PrintInformation();
#endif

				//cout << "numberOfPartitions: " << numberOfPartitions << " numOfCores: " << numOfCores << endl;
				//exit(0);

				return info;
			}

/*			
			PartitionInfo *ClusteredQueuingNetworkPartitioner::Partition(Vector<LogicalProcess *> *objects, Int32 &numberOfPartitions)
			{
				if(disposed)
					throw ObjectDisposedException("ClusteredQueuingNetworkPartitioner", "The ClusteredQueuingNetworkPartitioner has been disposed");

				if(objects == null)
					throw ArgumentNullException("objects", "objects is null");

				if(numberOfPartitions <= 0)
					throw ArgumentOutOfRangeException("numberOfPartitions", "numberOfPartitions is less than or equal to zero");

				Int32 numOfCores = numberOfPartitions;

				if(numOfRows < numberOfPartitions)
					numberOfPartitions = numOfRows;

				PartitionInfo *info = new PartitionInfo(numberOfPartitions);

				Vector< Vector<LogicalProcess*>* > partitions(numberOfPartitions);

				for(register Int32 i = 0 ; i < numberOfPartitions ; i++)
					partitions[i] = new Vector<LogicalProcess *>;

				// In this partitioning algorithm, we assume that the source and sink objects are in the end of the objects Vector.
				// In the algorithm for the clustered queuing network, we allocate possibly each queue network row to each CPU core.
				// If there is more than two cores, we allocate the source and sink objects into one dedicated core to maximize
				// the simulation concurrency. Otherwise, if there is only two cores then we allocate the source and sink objects
				// into last core.

				Int32 originalNumberOfPartitions = numberOfPartitions;

				// We take advantage of a dedicated core for source and sink objects.
				//if(originalNumberOfPartitions > 2)
				//	numberOfPartitions--;

				//printf("originalNumberOfPartitions: %d numberOfPartitions: %d\n", originalNumberOfPartitions, numberOfPartitions);

				// Distributes rows into CPU cores.
				Int32 perCore = numOfRows / numberOfPartitions;
				Int32 remainder = numOfRows % numberOfPartitions;

				// servers[i][j]
				for(register Int32 core = 0 ; core < numberOfPartitions ; core++)
				{
					for(register Int32 i = core * perCore ; i < core * perCore + perCore ; i++)
					{
						for(register Int32 j = 0 ; j < numOfServersPerRow ; j++)
						{
							(*objects)[i * numOfServersPerRow + j]->SetOwnerClusterID(core);
							partitions[core]->push_back((*objects)[i * numOfServersPerRow + j]);
							//(*objects)[i][j]->SetOwnerClusterID(core);
							//partitions[core]->push_back((*objects)[i][j]);
						}
					}
				}

				//printf("perCore: %d remainder: %d\n", perCore, remainder);

				// Distributes the remainder rows into the last core.
				if(remainder > 0)
				{
					for(register Int32 i = numberOfPartitions * perCore ; i < numberOfPartitions * perCore + remainder ; i++)
					{
						for(register Int32 j = 0 ; j < numOfServersPerRow ; j++)
						{
							(*objects)[i * numOfServersPerRow + j]->SetOwnerClusterID(numberOfPartitions - 1);
							partitions[numberOfPartitions - 1]->push_back((*objects)[i * numOfServersPerRow + j]);
							//(*objects)[i][j]->SetOwnerClusterID(numberOfPartitions - 1);
							//partitions[numberOfPartitions - 1]->push_back((*objects)[i][j]);
						}
					}
				}

				// Dispatches source and sink objects into the last core.

				/// Source object.
				(*objects)[objects->size() - 2]->SetOwnerClusterID(originalNumberOfPartitions - 1);
				partitions[originalNumberOfPartitions - 1]->push_back((*objects)[objects->size() - 2]);

				/// Sink object.
				(*objects)[objects->size() - 1]->SetOwnerClusterID(originalNumberOfPartitions - 1);
				partitions[originalNumberOfPartitions - 1]->push_back((*objects)[objects->size() - 1]);


				for(register Int32 i = 0 ; i < originalNumberOfPartitions ; i++)
					info->AddPartition(partitions[i]);

#ifdef __Parvicursor_xSim_Debug_Enable__
				info->PrintInformation();
#endif

				//cout << "numberOfPartitions: " << numberOfPartitions << " numOfCores: " << numOfCores << endl;
				//exit(0);

				return info;
			}
*/
			//----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
