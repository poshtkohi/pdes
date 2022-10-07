/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/


#ifndef __Parvicursor_xSim_LevelizedPartitioner_h__
#define __Parvicursor_xSim_LevelizedPartitioner_h__

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
#include <map>
using namespace std;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------

		// Forward declarations

		// class LogicalProcess;

		class LevelizedPartitioner : public Partitioner
		{
			private: class Vertx : public Object
			{
										// (lp, level)
				//public: vector< std::pair<Vertx *, Int32> > inputs; // input logical processes
				public: map< Vertx *, pair<Vertx *, Int32> > inputs; // input logical processes
										// (lp, level)
				public: Vector< pair<Vertx *, Int32> > outputs; // output logical processes
				public: LogicalProcess *owner;
				public: UInt32 level;
				public: bool visited;
			};
			/*---------------------fields-----------------*/
			private: bool disposed;
			private: LogicalProcess *root;
			private: Vertx **vertices;
			private: UInt32 numOfVertices;
			private: UInt32 maximumLevel;
			private: Vector< Vector<Vertx *> * > *blocks;
			/*---------------------methods----------------*/
			// LevelizedPartitioner Class constructor.
			public: LevelizedPartitioner(LogicalProcess *root);
			// LevelizedPartitioner Class destructor.
			public: ~LevelizedPartitioner();

			/// Overridden methods
			public: PartitionInfo *Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions);
			private: void BuildGraph(Vector<LogicalProcess *> *objects);
			private: void Levelize();
			private: void Visit(Vertx *v);
			/*--------------------------------------------*/

		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

