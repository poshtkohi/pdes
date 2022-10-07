/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/


#ifndef __Parvicursor_xSim_SpaceTimeGraph_h__
#define __Parvicursor_xSim_SpaceTimeGraph_h__

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

#include "LogicalProcess.h"

using namespace System;
using namespace System::Collections;
using namespace System::IO;

#include <vector>
#include <map>
#include <utility>
#include <list>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sstream>

using namespace std;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------

		// Forward declarations

		// class LogicalProcess;
		enum CreatedMode {CreatedBySendEvent, CreatedByGetNextEvent};

		class SpaceTimeGraph : public Object
		{
            private: enum Colour {White, Grey, Black};
			private: class Vertex : public Object
			{
				public: std::vector<UInt32> adjList;
				public: UInt32 sender;
				public: UInt32 receiver;
				public: UInt32 generator;
				public: Int64 time;
				public: Int64 id;
				public: Colour colour;
				public: CreatedMode mode;
				public: bool visited;
			};
			/*---------------------fields-----------------*/
			//private: LogicalProcess *root;
			public: std::vector<Vertex *> vertices;
			private: std::list<UInt32> sortedList;
			//private: Queue *q;
			private: UInt32 numOfVertices;
			private: UInt32 numOfEdges;
			private: bool generated;
			private: UInt32 numOfAssignedLogicalProcesses;
			/*---------------------methods----------------*/
			// SpaceTimeGraph Class constructor.
			public: SpaceTimeGraph();
			// SpaceTimeGraph Class destructor.
			public: ~SpaceTimeGraph();
			public: void AddVertex(CreatedMode mode, UInt32 generator, UInt32 sender, UInt32 receiver, Int64 time);
			public: void AddEdge(UInt32 u, UInt32 v);
			public: void GenerateDotGraph(const String &filename);
			private: void GenerateStgFileForParallelSimulation(const String &filename);
			public: static void LoadStgFileForParallelSimulation(const String &filename);
			private: void TopSort();
			private: void SortFromVertex(Vertex *v);
			public: void GeneratePriorities();
			public: void DFSUtil(Vertex *u);
			/*--------------------------------------------*/

		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

