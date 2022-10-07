/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_PdesTime_queue_lc_h__
#define __Parvicursor_PdesTime_queue_lc_h__


#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <general.h>
#include <StaticFunctions/StaticFunctions.h>
#include <System/BasicTypes/BasicTypes.h>

#include "PdesTime.h"
#include "RingBuffer.h"
#include "Globals.h"

using namespace System;

using namespace Parvicursor::xSim;
using namespace Parvicursor::xSim::Collections;

using namespace std;

//**************************************************************************************************************/

namespace Parvicursor
{
	namespace PS2
	{
		//----------------------------------------------------
		struct node_lc
		{
			PdesTime pri;
			void *data;

			inline node_lc &operator = (const node_lc &n)
			{
				//cout << "hello1node_lc" << endl;
				data = n.data;
				pri = n.pri;
				return *this;
			}
		};
		//----------------------------------------------------
		struct pri_queue_lc
		{
			UInt32 length;
			UInt32 allocations;
			UInt32 alloc;
			node_lc *array;
			UInt32 cluster;
		};
		//----------------------------------------------------
		struct pri_queue_lc *priq_new_lc(UInt32 size);
		void priq_delete_lc(struct pri_queue_lc *pq);
		// Returns 1 if length is zero
		int inline is_empty_lc(struct pri_queue_lc *pq)
		{
			return pq->length == 0;
		}
		// Returns the minimum element without removing it
		inline void *priq_top_lc(struct pri_queue_lc *pq, Out PdesTime *pri)
		{
			if(is_empty_lc(pq))
				return null;
			*pri = pq->array[0].pri;
			return pq->array[0].data;
		}
		//void priq_push(struct pri_queue_lc *pq, void *data, PdesTime pri);
		void priq_push_lc(struct pri_queue_lc *pq, In void *data, const PdesTime &pri);
		// Removes the minimum element and returns it
		void *priq_pop_lc(struct pri_queue_lc *pq, Out PdesTime *pri);
		void priq_test_lc();
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif


