/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/


#ifndef __Parvicursor_xSim_MetisPartitioner_h__
#define __Parvicursor_xSim_MetisPartitioner_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/Random/Random.h>
#include <System/Exception/Exception.h>
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
#include <map>
#include <utility>
using namespace std;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------

		// Forward declarations

		// class LogicalProcess;

		class MetisPartitioner : public Partitioner
		{
			private: class Vertex : public Object
			{
				public: std::map<Vertex *, Vertex *> adjList;
				public: LogicalProcess *owner;
			};
			/*---------------------fields-----------------*/
			private: bool disposed;
			//private: LogicalProcess *root;
			private: Vector<Vertex *> vertices;
			//private: Queue *q;
			private: UInt32 numOfVertices;
			private: UInt32 numOfEdges;
			/*---------------------methods----------------*/
			// MetisPartitioner Class constructor.
			public: MetisPartitioner(/*LogicalProcess *root*/);
			// MetisPartitioner Class destructor.
			public: ~MetisPartitioner();

			/// Overridden methods
			public: PartitionInfo *Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions);
			private: void BuildGraph(Vector<LogicalProcess *> *objects);
			private: void BuildNumberOfEdges(Vector<LogicalProcess *> *objects);
			private: void GenerateDotGraph(const String &filename);
			/*--------------------------------------------*/

		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

