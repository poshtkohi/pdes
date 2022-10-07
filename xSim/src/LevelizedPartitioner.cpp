/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "LevelizedPartitioner.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		LevelizedPartitioner::LevelizedPartitioner(LogicalProcess *root)
		{
			if(root == null)
				throw ArgumentNullException("root", "root is null");

			this->root = root;
			vertices = null;
			maximumLevel = 0;
			blocks = null;

			disposed = false;
		}
		//----------------------------------------------------
		LevelizedPartitioner::~LevelizedPartitioner()
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

				if(maximumLevel > 0 && blocks != null)
				{
					for(register UInt32 i = 0 ; i < maximumLevel + 1 ; i++)
						delete (*blocks)[i];

					delete blocks;
				}

				disposed = true;
			}
		}
		//----------------------------------------------------
		PartitionInfo *LevelizedPartitioner::Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions)
		{
			if(disposed)
				throw ObjectDisposedException("LevelizedPartitioner", "The LevelizedPartitioner has been disposed");

			if(objects == null)
				throw ArgumentNullException("objects", "objects is null");

			if(numberOfPartitions <= 0)
				throw ArgumentOutOfRangeException("numberOfPartitions", "numberOfPartitions is less than or equal to zero");

			BuildGraph(objects);
			Levelize();
			cout << "maximumLevel: " << maximumLevel << endl;
			/*for(register UInt64 i = 0 ; i < numOfVertices ; i++)
			{
				Vertx *v = vertices[i];
				cout << v->owner->GetName().get_BaseStream() << " outputs: " << v->outputs.size() << " inputs " << v->inputs.size() << " level: " << v->level << endl;
			}*/

			PartitionInfo *info = new PartitionInfo();

			Vector< Vector<LogicalProcess*>* > partitions(numberOfPartitions);

			for(register UInt32 i = 0 ; i < numberOfPartitions ; i++)
				partitions[i] = new Vector<LogicalProcess *>;

			/*for(register Int32 level = 0, n = 0; level < maximumLevel + 1 ; level++)
			{
				vector<Vertx *> *block = (*blocks)[level];
				for(register Int32 i = 0 ; i < block->size() ; i++, n++) // Random
				{
					if(n == numberOfPartitions)
						n = 0;
					LogicalProcess *lp = (*block)[i]->owner;
					lp->SetOwnerClusterID(n);
					partitions[n]->push_back(lp);
				}
			}*/

			UInt32 perCore = (maximumLevel + 1) / numberOfPartitions ;
			UInt32 remainder = (maximumLevel + 1) % numberOfPartitions;

			// Distributes objects into cores.
			for(register UInt32 core = 0 ; core < numberOfPartitions ; core++)
			{
				for(register UInt32 level = core * perCore ; level < core * perCore + perCore ; level++)
				{
					Vector<Vertx *> *block = (*blocks)[level];
					for(register UInt32 i = 0 ; i < block->Size() ; i++)
					{
						//cout << (*block)[i]->owner->GetName().get_BaseStream() << ", ";
						LogicalProcess *lp = (*block)[i]->owner;
						lp->SetOwnerClusterID(core);
						partitions[core]->PushBack(lp);
					}
				}
			}

			// Distributes the remainder objects into the last core.
			if(remainder > 0)
			{
				for(register UInt32 level = numberOfPartitions * perCore ; level < maximumLevel + 1 ; level++)
				{
					Vector<Vertx *> *block = (*blocks)[level];
					for(register UInt32 i = 0 ; i < block->Size() ; i++)
					{
						LogicalProcess *lp = (*block)[i]->owner;
						lp->SetOwnerClusterID(numberOfPartitions - 1);
						partitions[numberOfPartitions - 1]->PushBack(lp);
					}
				}
				cout << "remainder: " << remainder << endl;
			}


			for(register UInt32 i = 0 ; i < numberOfPartitions ; i++)
				info->AddPartition(partitions[i]);

#if defined __Parvicursor_xSim_Debug_Enable__
			info->PrintInformation();
#endif
			//exit(0);
			return info;
		}
		//----------------------------------------------------
		void LevelizedPartitioner::BuildGraph(Vector<LogicalProcess *> *objects)
		{
			numOfVertices = objects->Size();
			vertices = new Vertx *[objects->Size()];

			for(register UInt32 i = 0 ; i < objects->Size() ; i++)
			{
				Vertx *v = new Vertx();
				v->owner = (*objects)[i];
				v->level = -1;
				v->visited = false;
				vertices[v->owner->GetID()] = v;
			}

			for(register UInt32 i = 0 ; i < numOfVertices ; i++)
			{
				Vertx *v = vertices[i];

				for(register UInt32 j = 0 ; j < v->owner->GetOutputLpCount() ; j++)
				{
					LogicalProcess *outputLp = v->owner->GetLogicalProcessById( v->owner->GetOutputLpId(j) );

					std::pair<Vertx *, Int32> out( vertices[outputLp->GetID()] , -1);
					v->outputs.PushBack(out);

					Vertx *w = vertices[outputLp->GetID()];
					std::pair<Vertx *, Int32> in( vertices[v->owner->GetID()] , -1);
					//w->inputs.push_back(in);
					w->inputs[v] = in;
				}
			}
		}
		//----------------------------------------------------
		void LevelizedPartitioner::Levelize()
		{
			Vertx *root_ = vertices[ root->GetID() ];

			for(register UInt32 i = 0 ; i < root_->outputs.Size() ; i++)
			{
				std::pair<Vertx *, Int32> p = root_->outputs[i];
				Vertx *w = p.first;
				w->inputs[root_].second = 0;
			}

			Visit(root_);
			root_->level = 0;

			if(maximumLevel > 0)
			{
				blocks = new Vector< Vector<Vertx *> * >(maximumLevel + 1);
				for(register UInt32 i = 0 ; i < maximumLevel + 1; i++)
					(*blocks)[i] = new Vector<Vertx *>;

				cout << "blocks->size(): " << blocks->Size() << endl;

				for(register UInt32 i = 0 ; i < numOfVertices ; i++)
				{
					Vertx *v = vertices[i];
					//cout << "i: " << i << " v->level: " << v->level << endl;
					(*blocks)[v->level]->PushBack(v);
				}

				for(register UInt32 level = 0 ; level < maximumLevel + 1 ; level++)
				{
					cout << "-------------------------------------------\nLevel " << level << " ";
					Vector<Vertx *> *block = (*blocks)[level];
					for(register UInt32 i = 0 ; i < block->Size() ; i++)
					{
						cout << (*block)[i]->owner->GetName().get_BaseStream() << ", ";
					}
					cout << endl;
				}
				cout << "-------------------------------------------" << endl;
			}
		}
		//----------------------------------------------------
		void LevelizedPartitioner::Visit(Vertx *v)
		{
			//if(v->level >= 0)
			//	return;

			// Scan all inputs and select the maximum. If at least one inputs
			// has not yet been levelized, we must break from the loop and continue
			// visiting the next vertex recursively.
			v->visited = true;
			//cout << "v: "<< v->owner->GetName().get_BaseStream() << endl;
			for(register UInt32 i = 0 ; i < v->outputs.Size() ; i++)
			{
				Vertx *w = v->outputs[i].first;
				//if(!w->visited)
				//
				if(w->level < 0)
				{
					// Computes the level for current vertex.
					Int32 maximum = -1;
					map< Vertx *, pair<Vertx *, Int32> >::iterator it;
					for(it = w->inputs.begin() ; it != w->inputs.end() ; it++)
					{
						// first=key, second=value
						pair<Vertx *, Int32> p = it->second;
						cout << it->first << ':' << it->second.second << endl;
						if(p.second < 0)
						{
							maximum = -1;
							break;
						}

						if(p.second > maximum)
							maximum = p.second;
					}
					if(maximum >= 0)
					{
						UInt32 level = maximum + 1;
						std::cout << "level " << level << std::endl;///
						w->level = level;
						if(level > maximumLevel)
							maximumLevel = level;
						// Sets the destination input levels of the w vertex.
						for(register UInt32 i = 0 ; i < w->outputs.Size() ; i++)
						{
							std::pair<Vertx *, Int32> p = w->outputs[i];
							Vertx *u = p.first;
							u->inputs[w].second = level;
						}
					}
				}
				Visit(w);
			}
			//if(v->inputs.size() == 0)
			//	v->level = 0;
			/*1  procedure DFS(G,v):
			2      label v as discovered
				3      for all edges from v to w in G.adjacentEdges(v) do
			4          if vertex w is not labeled as discovered then
				5              recursively call DFS(G,w)
				*/
		}
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
