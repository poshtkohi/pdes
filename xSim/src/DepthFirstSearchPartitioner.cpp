/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "DepthFirstSearchPartitioner.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		DepthFirstSearchPartitioner::DepthFirstSearchPartitioner(LogicalProcess *root)
		{
			if(root == null)
				throw ArgumentNullException("root", "root is null");

			this->root = root;
			currentCore = 0;
			numberOfPaths = 0;

			disposed = false;
		}
		//----------------------------------------------------
		DepthFirstSearchPartitioner::~DepthFirstSearchPartitioner()
		{
			if(!disposed)
			{
				disposed = true;
			}
		}
		//----------------------------------------------------
		PartitionInfo *DepthFirstSearchPartitioner::Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions)
		{
			if(disposed)
				throw ObjectDisposedException("DepthFirstSearchPartitioner", "The DepthFirstSearchPartitioner has been disposed");

			if(objects == null)
				throw ArgumentNullException("objects", "objects is null");

			if(numberOfPartitions <= 0)
				throw ArgumentOutOfRangeException("numberOfPartitions", "numberOfPartitions is less than or equal to zero");

			DFS_ComputeNumberOfPaths(root);
			cout << "NumberOfPaths: " << numberOfPaths << " numberOfPartitions: " << numberOfPartitions << endl;
			for(register UInt32 i = 0 ; i < objects->Size() ; i++)
				(*objects)[i]->visited = false;
			if(numberOfPaths < numberOfPartitions)
				numberOfPartitions = numberOfPaths;
			numberOfCores = numberOfPartitions;
			cout << "numberOfCores: " << numberOfCores << endl;
			//exit(0);
			PartitionInfo *info = new PartitionInfo();

			Vector< Vector<LogicalProcess*>* > partitions(numberOfPartitions);
			//cores.reserve(numberOfCores);
			for(register UInt32 i = 0 ; i < numberOfCores ; i++)
				cores[i] = 0;

			for(register UInt32 i = 0 ; i < numberOfPartitions ; i++)
				partitions[i] = new Vector<LogicalProcess *>;

			/*for(register Int32 i = 0, n = 0; i < objects->size() ; i++, n++)
			{
				if(n == numberOfPartitions)
					n = 0;
				(*objects)[i]->SetOwnerClusterID(n);
				partitions[n]->push_back((*objects)[i]);
			}*/

			DFS(root, &partitions);

			//exit(0);

			for(register UInt32 i = 0 ; i < numberOfPartitions ; i++)
				info->AddPartition(partitions[i]);

#if defined __Parvicursor_xSim_Debug_Enable__
			info->PrintInformation();
#endif
			//exit(0);
			return info;
		}
		//----------------------------------------------------
		void DepthFirstSearchPartitioner::DFS(LogicalProcess *v, Vector< Vector<LogicalProcess*>* > *partitions)
		{
			// This is a monitor object (PO).
			if(v->GetOutputLpCount() == 0)
			{
				if(!v->visited)
				{
					v->visited = true;
					v->SetOwnerClusterID(currentCore);
					(*partitions)[currentCore]->PushBack(v);
					//cout << "DepthFirstSearchPartitioner::DFS1x: " << v->GetName().get_BaseStream() << endl;
				}

				currentCore++;
				//cout << "currentCore: " << currentCore << " numberOfCores: " << numberOfCores << endl;
				if(currentCore == numberOfCores)
					currentCore = 0;

				/*cores[currentCore] = cores[currentCore] + 1;
				Int32 index = 0, minimum = -1;
				for(register Int32 i = 0 ; i < numberOfCores ; i++)
				{
					if(i == 0)
					{
						minimum = cores[i];
						index = i;
						continue;
					}
					else if(cores[i] < minimum)
					{
						minimum = cores[i];
						index = i;
					}
				}
				currentCore = index;*/

				//Random rnd = Random(currentCore);
				//currentCore = rnd.Next(0, numberOfCores - 1);
				//cout << "currentCore: " << currentCore << " numberOfCores: " << numberOfCores << endl;
				//Thread::Sleep(1000);
				return;
			}

			v->SetOwnerClusterID(currentCore);
			(*partitions)[currentCore]->PushBack(v);
			v->visited = true;
			///cout << "DepthFirstSearchPartitioner::DFS2: " << v->GetName().get_BaseStream() << " currentCore: " << currentCore << endl;
			for(register UInt32 i = 0 ; i < v->GetOutputLpCount() ; i++)
			{
				LogicalProcess *w = v->GetLogicalProcessById(v->GetOutputLpId(i));
				if(!w->visited || w->GetOutputLpCount() == 0)
					DFS(w, partitions);
			}
		}
		//----------------------------------------------------
		void DepthFirstSearchPartitioner::DFS_ComputeNumberOfPaths(LogicalProcess *v)
		{
			// This is a monitor object (PO).
			if(v->GetOutputLpCount() == 0)
			{
				//cout << "hello" << endl;
				numberOfPaths++;
				return;
			}
			v->visited = true;
			//cout << "DepthFirstSearchPartitioner::DFS: " << v->GetName().get_BaseStream() << " v->GetOutputLpCount(): " << v->GetOutputLpCount() << endl;
			for(register UInt32 i = 0 ; i < v->GetOutputLpCount() ; i++)
			{
				LogicalProcess *w = v->GetLogicalProcessById(v->GetOutputLpId(i));
				if(!w->visited)
					DFS_ComputeNumberOfPaths(w);
			}
		}
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
