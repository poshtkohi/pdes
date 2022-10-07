/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/


#ifndef __Parvicursor_xSim_StronglyConnectedComponentPartitioner_h__
#define __Parvicursor_xSim_StronglyConnectedComponentPartitioner_h__

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
#include <System.IO/IOException/IOException.h>

#include "Partitioner.h"
#include "LogicalProcess.h"

using namespace System;
using namespace System::Collections;
using namespace System::IO;

//#include <vector>
//using namespace std;
#include <iostream>
#include <sstream>
#include <fstream>
#include <sstream>
#include <map>
#include <utility>
#include <stack>
using namespace std;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------

		// Forward declarations

		// class LogicalProcess;

		class StronglyConnectedComponentPartitioner : public Partitioner
		{
			private: class Vertex : public Object
			{
				public: std::map<Vertex *, Vertex *> adjList;
				public: LogicalProcess *owner;
				public: bool visited;
			};
			/*---------------------fields-----------------*/
			private: bool disposed;
			//private: LogicalProcess *root;
			private: Vector<Vertex *> vertices;
			//private: Queue *q;
			private: UInt32 numOfVertices;
			private: UInt32 numOfEdges;
			/*---------------------methods----------------*/
			// StronglyConnectedComponentPartitioner Class constructor.
			public: StronglyConnectedComponentPartitioner(/*LogicalProcess *root*/);
			// StronglyConnectedComponentPartitioner Class destructor.
			public: ~StronglyConnectedComponentPartitioner();

			/// Overridden methods
			public: PartitionInfo *Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions);
			/// Other methods
			private: void BuildGraph(Vector<LogicalProcess *> *objects);
            private: void PrintGraph();
			private: void GenerateDotGraph(const String &filename, Vector<Vertex *> &vertices);
			/*--------------------------------------------*/

		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

