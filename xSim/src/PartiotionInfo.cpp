/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "PartitionInfo.h"
#include "LogicalProcess.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        //----------------------------------------------------
		PartitionInfo::PartitionInfo()
		{
			numberOfPartitions = 0;

			disposed = false;
		}
		//----------------------------------------------------
		PartitionInfo::~PartitionInfo()
		{
			if(!disposed)
			{
				for(register Int32 i = 0 ; i < partitions.Size() ; i++)
					delete partitions[i];

				disposed = true;
			}
		}
		//----------------------------------------------------
		Int32 PartitionInfo::GetNumberOfPartitions()
		{
			if(disposed)
				throw ObjectDisposedException("PartitionInfo", "The PartitionInfo has been disposed");

			return numberOfPartitions;
		}
		//----------------------------------------------------
		Vector<LogicalProcess *> *PartitionInfo::GetPartitionByIndex(Int32 index)
		{
			if(disposed)
				throw ObjectDisposedException("PartitionInfo", "The PartitionInfo has been disposed");

			if(index < 0 || index > numberOfPartitions - 1)
				throw ArgumentOutOfRangeException("index", "index must be greater than or equal to zero and be less than GetNumberOfPartitions()");

			return partitions[index];
		}
		//----------------------------------------------------
		void PartitionInfo::AddPartition(Vector<LogicalProcess *> *partition)
		{
			if(disposed)
				throw ObjectDisposedException("PartitionInfo", "The PartitionInfo has been disposed");

			if(partition == null)
				throw ArgumentNullException("partition", "partition is null");

            numberOfPartitions++;
			partitions.PushBack(partition);
		}
		//----------------------------------------------------
		void PartitionInfo::PrintInformation()
		{
			cout << "------------------------------------\nPartitioning Information:\nNumber of Partitions: " << GetNumberOfPartitions() << "\n";

            for(register Int32 i = 0 ; i < GetNumberOfPartitions() ; i++)
			{
				Vector<LogicalProcess *> *partition = GetPartitionByIndex(i);
				cout << "The Number of Objects in Partition " << i << " is : " << partition->Size() << endl;
			}

			/*for(register Int32 i = 0 ; i < GetNumberOfPartitions() ; i++)
			{
				Vector<LogicalProcess *> *partition = GetPartitionByIndex(i);
				cout << "Objects in Partition " << i << " are : ";
				for(register Int32 j = 0 ; j < partition->Size() ; j++)
					//cout << (*partition)[j]->GetID() << ", ";
					cout << (*partition)[j]->GetName().get_BaseStream() << ", ";

				cout << endl;
			}*/

			/*for(register Int32 i = 0 ; i < GetNumberOfPartitions() ; i++)
			{
				Vector<LogicalProcess *> *partition = GetPartitionByIndex(i);
				for(register Int32 j = 0 ; j < partition->Size() ; j++)
				{
					cout << "Output LPs of the LP " << (*partition)[j]->GetID() << " are ";
					for(register Int32 k = 0 ; k < (*partition)[j]->GetOutputLpCount(); k++)
						cout << (*partition)[j]->GetOutputLpId(k) << ", ";
					cout << endl;
				}
			}*/
		}
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
