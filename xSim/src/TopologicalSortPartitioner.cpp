/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "TopologicalSortPartitioner.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		TopologicalSortPartitioner::TopologicalSortPartitioner(/*LogicalProcess *root*/)
		{
			/*if(root == null)
				throw ArgumentNullException("root", "root is null");*/

			//this->root = root;
			vertices = null;
			//q = null;
			sortedList = null;

			disposed = false;
		}
		//----------------------------------------------------
		TopologicalSortPartitioner::~TopologicalSortPartitioner()
		{
			if(!disposed)
			{
				if(vertices != null)
				{
					for(register UInt32 i = 0 ; i < numOfVertices ; i++)
						if(vertices[i] != null)
							delete vertices[i];

					delete vertices;
					vertices = null;
				}

				//if(q != null)
				//	delete q;
				if(sortedList != null)
					delete sortedList;

				disposed = true;
			}
		}
		//----------------------------------------------------
		PartitionInfo *TopologicalSortPartitioner::Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions)
		{
			if(disposed)
				throw ObjectDisposedException("TopologicalSortPartitioner", "The TopologicalSortPartitioner has been disposed");

			if(objects == null)
				throw ArgumentNullException("objects", "objects is null");

			if(numberOfPartitions <= 0)
				throw ArgumentOutOfRangeException("numberOfPartitions", "numberOfPartitions is less than or equal to zero");

			BuildGraph(objects);
			TopSort();

			/*PartitionInfo *info = new PartitionInfo(numberOfPartitions);

			Vector< Vector<LogicalProcess*>* > partitions(numberOfPartitions);

			for(register Int32 i = 0 ; i < numberOfPartitions ; i++)
				partitions[i] = new Vector<LogicalProcess *>;

			LogicalProcess *driver;

			cout << sortedList->get_Count() << endl;

			for(Int32 i = 0 ; i < sortedList->get_Count() ; i++)
			{
				LogicalProcess *lp = ((Vertex *)sortedList->get_Value(i))->owner;
				if(lp->GetName() == "driver")
				{
					cout << "driver" << endl;
					driver = lp;
					sortedList->RemoveAt(i);
					break;
				}
			}
			cout << sortedList->get_Count() << endl;

			Int32 perCore = sortedList->get_Count() / (numberOfPartitions - 1 );
			Int32 remainder = sortedList->get_Count()  % (numberOfPartitions - 1);

			// Distributes objects into cores.
			for(register Int32 core = 0 ; core < numberOfPartitions - 1 ; core++)
			{
				for(register Int32 index = core * perCore ; index < core * perCore + perCore ; index++)
				{
					LogicalProcess *lp = ((Vertex *)sortedList->get_Value(index))->owner;
					lp->SetOwnerClusterID(core);
					partitions[core]->PushBack(lp);
				}
			}

			// Distributes the remainder objects into the last core.
			if(remainder > 0)
			{
				for(register Int32 index = (numberOfPartitions - 1) * perCore ; index < objects->Size() - 1; index++)
				{
					cout << "index " << index << endl;
					LogicalProcess *lp = ((Vertex *)sortedList->get_Value(index))->owner;
					lp->SetOwnerClusterID(numberOfPartitions - 1 - 1);
					partitions[numberOfPartitions - 1 - 1]->PushBack(lp);
				}
			}

			driver->SetOwnerClusterID(numberOfPartitions - 1);
			partitions[numberOfPartitions - 1]->PushBack(driver);

			for(register Int32 i = 0 ; i < numberOfPartitions ; i++)
				info->AddPartition(partitions[i]);*/

			PartitionInfo *info = new PartitionInfo();

			Vector< Vector<LogicalProcess*>* > partitions(numberOfPartitions);

			for(register UInt32 i = 0 ; i < numberOfPartitions ; i++)
				partitions[i] = new Vector<LogicalProcess *>;

			UInt32 perCore = sortedList->get_Count() / numberOfPartitions;
			UInt32 remainder = sortedList->get_Count() % numberOfPartitions;

			// Distributes objects into cores.
			for(register UInt32 core = 0 ; core < numberOfPartitions ; core++)
			{
				for(register UInt32 index = core * perCore ; index < core * perCore + perCore ; index++)
				{
					LogicalProcess *lp = ((Vertex *)sortedList->get_Value(index))->owner;
					lp->SetOwnerClusterID(core);
					partitions[core]->PushBack(lp);
				}
			}

			// Distributes the remainder objects into the last core.
			if(remainder > 0)
			{
				for(register UInt32 index = numberOfPartitions * perCore ; index < objects->Size() ; index++)
				{
					LogicalProcess *lp = ((Vertex *)sortedList->get_Value(index))->owner;
					lp->SetOwnerClusterID(numberOfPartitions - 1);
					partitions[numberOfPartitions - 1]->PushBack(lp);
				}
			}

			for(register UInt32 i = 0 ; i < numberOfPartitions ; i++)
				info->AddPartition(partitions[i]);

//#if defined __Parvicursor_xSim_Debug_Enable__
			info->PrintInformation();
//#endif
			//exit(0);
			return info;
		}
		//----------------------------------------------------
		void TopologicalSortPartitioner::BuildGraph(Vector<LogicalProcess *> *objects)
		{
			numOfVertices = objects->Size();
			vertices = new Vertex *[objects->Size()];
			sortedList = new ArrayList();
			//q = new Queue();

			for(register UInt32 i = 0 ; i < objects->Size() ; i++)
			{
				Vertex *v = new Vertex();
				v->owner = (*objects)[i];
				v->colour = White;
				vertices[v->owner->GetID()] = v;
			}

			/*for(register UInt64 i = 0 ; i < numOfVertices ; i++)
			{
				Vertex *v = vertices[i];

				for(register Int32 j = 0 ; j < v->owner->GetOutputLpCount() ; j++)
				{
					LogicalProcess *outputLp = v->owner->GetLogicalProcessById( v->owner->GetOutputLpId(j) );

					std::pair<LogicalProcess *, Int32> out(outputLp, -1);
					v->outputs.push_back(out);

					Vertex *w = vertices[outputLp->GetID()];
					std::pair<LogicalProcess *, Int32> in(v->owner, -1);
					w->inputs.push_back(in);
				}
			}*/

			for(register UInt32 i = 0 ; i < numOfVertices ; i++)
			{
				Vertex *v = vertices[i];

				for(register UInt32 j = 0 ; j < v->owner->GetOutputLpCount() ; j++)
				{
					v->adjList.PushBack( vertices[ v->owner->GetOutputLpId(j) ] );
				}
			}


			/*for(register UInt64 i = 0 ; i < numOfVertices ; i++)
			{
				Vertex *v = vertices[i];
				cout << v->owner->GetName().get_BaseStream() << " outputs: " << v->outputs.size() << " inputs " << v->inputs.size() << endl;
			}*/
		}
		//----------------------------------------------------
		void TopologicalSortPartitioner::TopSort()
		{
			for(register UInt32 i = 0 ; i < numOfVertices ; i++)
			{
				Vertex *v = vertices[i];
				if(v->colour == White)
					SortFromVertex(v);
			}

			/*cout << "\nSorted list is ";
			for(register Int32 i = 0 ; i < sortedList->get_Count() ; i++)
			{
				Vertex  *v = (Vertex *)sortedList->get_Value(i);
				cout << v->owner->GetName().get_BaseStream() << ", ";
			}*/

			/*Vertex *temp = null;
			while((temp = (Vertex *)q->Dequeue()) != null)
			{
				cout << temp->owner->GetName().get_BaseStream() << ", ";
			}*/

			//cout << endl;
		}
		//----------------------------------------------------
		void TopologicalSortPartitioner::SortFromVertex(Vertex *v)
		{
			v->colour = Grey;
			for(register UInt32 i = 0 ; i < v->adjList.Size() ; i++)
			{
				Vertex *w = v->adjList[i];
				if(w->colour == White)
					SortFromVertex(w);
				else if(w->colour == Grey)
				{
					cout << "The graph of the model has a cycle" << endl;
					exit(0);
				}
			}

			v->colour = Black;
			//q->Enqueue(v);
			sortedList->Add(v);
		}
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
