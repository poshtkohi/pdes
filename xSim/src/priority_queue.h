/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_PS2_priority_queue_h__
#define __Parvicursor_PS2_priority_queue_h__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <general.h>
#include <StaticFunctions/StaticFunctions.h>
#include <System/BasicTypes/BasicTypes.h>

#include "RingBuffer.h"
#include "Globals.h"

using namespace System;

using namespace Parvicursor::xSim::Collections;
using namespace Parvicursor::PS2;

using namespace std;

//**************************************************************************************************************/

namespace Parvicursor
{
	namespace PS2
	{
		//----------------------------------------------------
		/*
			A Binary Heap implementation
		*/
		//----------------------------------------------------
		/* Binary Heap struct with a length and set length array */
		struct ps2_priority
		{
			UInt64 timestamp;
#if defined __Delta_Enabled1__
			UInt64 delta;
			//UInt32 delta;
#endif
/*			inline bool operator < (const ps2_priority &pri)
			{
				//cout << "hello*1" << endl;
				if(timestamp < pri.timestamp)
					return true;
#if defined __Delta_Enabled1__
				else if(timestamp == pri.timestamp && delta < pri.delta)
					return true;
#endif
				else
					return false;
			}*/
            public: inline friend bool operator < (const ps2_priority &a, const ps2_priority &b)
			{
                //return a.timestamp < b.timestamp;
				//	return true;
                if(a.timestamp < b.timestamp)
					return true;
#if defined __Delta_Enabled1__
                else if(a.timestamp == b.timestamp && a.delta < b.delta)
					return true;
#endif
				else
					return false;
			}

			inline ps2_priority &operator = (const ps2_priority &pri)
			{
				//cout << "hello1" << endl;
				timestamp = pri.timestamp;
#if defined __Delta_Enabled1__
				delta = pri.delta;
#endif
				return *this;
			}

			/*inline ps2_priority &operator = (ps2_priority &pri)
			{
				//cout << "hello2" << endl;
				timestamp = pri.timestamp;
#if defined __Delta_Enabled1__
				delta = pri.delta;
#endif
				return *this;
			}

			inline ps2_priority *operator = (const ps2_priority *pri)
			{
				//cout << "hello3" << endl;
				timestamp = pri->timestamp;
#if defined __Delta_Enabled1__
				delta = pri->delta;
#endif
				return this;
			}

			inline ps2_priority *operator = (ps2_priority *pri)
			{
				cout << "hello4" << endl;
				timestamp = pri->timestamp;
#if defined __Delta_Enabled1__
				delta = pri->delta;
#endif
				return this;
			}*/

/*			inline bool operator > (const ps2_priority &pri)
			{
				//cout << "hello*2" << endl;
				if(timestamp > pri.timestamp)
					return true;
#if defined __Delta_Enabled1__
				else if(timestamp == pri.timestamp && delta > pri.delta)
					return true;
#endif
				else
					return false;
			}*/

			/*inline bool operator == (const ps2_priority &pri)
			{
#if defined __Delta_Enabled1__
				if(timestamp == pri.timestamp && delta == pri.delta)
					return true;
#else
				if(timestamp == pri.timestamp)
					return true;
#endif
				else
					return false;
			}

			inline bool operator != (const ps2_priority &pri)
			{
#if defined __Delta_Enabled1__
				if(timestamp != pri.timestamp && delta != pri.delta)
					return true;
#else
				if(timestamp != pri.timestamp)
					return true;
#endif
				else
					return false;
			}*/
		};
		//----------------------------------------------------
		struct node
		{
			ps2_priority pri;
			void *data;

			inline node &operator = (const node &n)
			{
				//cout << "hello1node" << endl;
				data = n.data;
				pri = n.pri;
				return *this;
			}

			/*inline node &operator = (node &n)
			{
				//cout << "hello2node" << endl;
				data = n.data;
				pri = n.pri;
				return *this;
			}*/
		};
		//----------------------------------------------------
		struct pri_queue
		{
#if defined __Delta_Enabled1__
			UInt64 counter;
			//UInt32 counter;
#endif
			UInt32 length;
			UInt32 allocations;
			UInt32 alloc;
			node *array;

#if defined __Delta_FIFO_Enabled__
			RingBuffer<void *> *fifo;
			// Current timestamp
			ps2_priority cts;
#endif
		};
		//----------------------------------------------------
		struct pri_queue *priq_new(UInt32 size);
		void priq_delete(struct pri_queue *pq);
		// Returns 1 if length is zero
		int inline is_empty(struct pri_queue *pq)
		{
			return pq->length == 0;
		}
		// Returns the minimum element without removing it
		void *priq_top(struct pri_queue *pq, ps2_priority *pri);
		inline void *priq_top(struct pri_queue *pq, ps2_priority *pri)
		{
#if defined __Delta_FIFO_Enabled__
			RingBuffer<void *> *fifo = pq->fifo;
			if(fifo->GetCount() != 0)
			{
				*pri = pq->cts;
				return fifo->Peek();
			}
#endif
			if(is_empty(pq))
				return null;
			*pri = pq->array[0].pri;
/*#if defined __Delta_Enabled1__
			std::cout << "top pri (" << pri->timestamp << ", " << pri->delta << ") (timestamp, delta)" << endl;
#else
			std::cout << "pop pri (" << pri->timestamp << ") (timestamp)" << endl;
#endif*/
			return pq->array[0].data;
		}
		//void priq_push(struct pri_queue *pq, void *data, ps2_priority pri);
		void priq_push(struct pri_queue *pq, void *data, UInt64 timestamp);
		// Removes the minimum element and returns it
		void *priq_pop(struct pri_queue *pq, ps2_priority *pri);
		void priq_test();
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif


