/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/


#ifndef __Parvicursor_xSim_TopologicalSortPartitioner_h__
#define __Parvicursor_xSim_TopologicalSortPartitioner_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/Random/Random.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>
#include <System/ArgumentException/ArgumentNullException.h>
#include <System.Collections/Queue/Queue.h>
#include <System.Collections/ArrayList/ArrayList.h>

#include "Partitioner.h"
#include "LogicalProcess.h"

using namespace System;
using namespace System::Collections;

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

		class TopologicalSortPartitioner : public Partitioner
		{
			private: enum Colour {White, Grey, Black};
			private: class Vertex : public Object
			{
										// (lp, level)
				//public: vector< std::pair<LogicalProcess *, Int32> > inputs; // input logical processes
										// (lp, level)
				//public: vector< std::pair<LogicalProcess *, Int32> > outputs; // output logical processes
				public: Vector<Vertex *> adjList;
				public: LogicalProcess *owner;
				public: Colour colour;
			};
			/*---------------------fields-----------------*/
			private: bool disposed;
			//private: LogicalProcess *root;
			private: Vertex **vertices;
			//private: Queue *q;
			private: ArrayList *sortedList;
			private: UInt32 numOfVertices;
			/*---------------------methods----------------*/
			// TopologicalSortPartitioner Class constructor.
			public: TopologicalSortPartitioner(/*LogicalProcess *root*/);
			// TopologicalSortPartitioner Class destructor.
			public: ~TopologicalSortPartitioner();

			/// Overridden methods
			public: PartitionInfo *Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions);
			private: void BuildGraph(Vector<LogicalProcess *> *objects);
			private: void TopSort();
			private: void SortFromVertex(Vertex *v);
			/*--------------------------------------------*/

		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

