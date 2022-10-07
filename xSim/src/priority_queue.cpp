/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "priority_queue.h"
#include "EventInterface.h"
#include "LogicalProcess.h"

using namespace Parvicursor::xSim;

//UInt64 delta_events = 0;
//UInt64 timed_events = 0;

//**************************************************************************************************************//
namespace Parvicursor
{
	namespace PS2
	{
		//----------------------------------------------------
		struct pri_queue *priq_new(UInt32 size)
		{
			if(size == 0)
				size = 128;
			//printf("priq_new::sizeof: %d\n", sizeof(pri_queue) );
			pri_queue *pq = (pri_queue *)::malloc(sizeof(pri_queue));
			pq->array = (node *)::malloc(sizeof(node) * size);
			pq->alloc = size;
			pq->length = 0;
			//cout << "size " << size << endl;
#if defined __Delta_Enabled1__
			pq->counter = 0;
#endif

#if defined __Delta_FIFO_Enabled__
			pq->fifo = new RingBuffer<void *>(size);
			pq->cts.timestamp = 0;
			//printf("__Delta_FIFO_Enabled__ pq->cts %d fifo.count() %d\n", pq->cts.timestamp, pq->fifo.GetCount());
			//cout << "fifo.count() " << pq->fifo->GetCount() << endl;
#endif

			return pq;
		}
		//----------------------------------------------------
		void priq_delete(struct pri_queue *pq)
		{
#if defined __Delta_FIFO_Enabled__
			delete pq->fifo;
#endif
			::free(pq->array);
			::free(pq);

			//cout << "Delta events: " << delta_count << endl;
			//cout << "Timed events: " << timed_events << endl;
		}
		//----------------------------------------------------
		// Returns the minimum element without removing it
		/*void *priq_top(struct pri_queue *pq, ps2_priority *pri)
		{
#if defined __Delta_FIFO_Enabled__
			if(pq->fifo->GetCount() != 0)
			{
				*pri = pq->cts;
				return (void *)pq->fifo->Peek();
			}
#endif
			if(is_empty(pq))
				return null;
			*pri = pq->array[0].pri;
			return pq->array[0].data;
		}*/
		//----------------------------------------------------
		// Percolates the hole from the new position and fills the pri once no elements above are less
		//void priq_push(struct pri_queue *pq, void *data, ps2_priority pri)
		//Int32 q = 0;
		void priq_push(struct pri_queue *pq, void *data, UInt64 timestamp)
		{
			ps2_priority pri;
			pri.timestamp = timestamp;

#if defined __Delta_FIFO_Enabled__
			if(timestamp == pq->cts.timestamp)
			{
				//delta_count++;
				pq->fifo->Enqueue(data);
				///cout << "pq->fifo->length: " << pq->fifo->GetCount() << endl;
				return;
			}
#endif

            //priq_push_internal(pq, data, pri);

            if(pq->length >= pq->alloc)
			{
                //q++;
				pq->alloc *= 2;
				pq->array = (node *)realloc(pq->array, sizeof(node) * pq->alloc);
				pq->allocations++;
				//std::cout << "pq->length " << pq->length << "  pq->alloc " <<  pq->alloc << " q " << q << endl; exit(0);
			}

#if defined __Delta_Enabled1__
			pri.delta = pq->counter;
			pq->counter++;
#endif

			register UInt32 hole = ++pq->length;

			//for( ; hole > 1 && pri < pq->array[ hole / 2 - 1].pri ; hole /= 2 )\
				pq->array[ hole - 1] = pq->array[ hole / 2 - 1];

			register UInt32 _hole_2 = hole/2 - 1;\
			for( ; hole > 1 && pri < pq->array[_hole_2].pri ; hole /= 2, _hole_2 = hole/2 - 1)\
				pq->array[ hole - 1] = pq->array[ _hole_2];

			pq->array[ hole - 1 ].pri = pri;
			pq->array[ hole - 1 ].data = data;

			//cout << "pq->length: " << pq->length << endl;

/*#if defined __Delta_Enabled1__
			std::cout << "push pri (" << pri.timestamp << ", " << pri.delta << ") (timestamp, delta)" << endl;
			//printf("push counter: %d\n", pq->counter);
#else
			std::cout << "push pri (" << pri.timestamp << ") (timestamp)" << endl;
#endif*/

		}
		//----------------------------------------------------
		// Percolates the hole down after removing the top element
		void percolate_down(struct pri_queue *pq, UInt32 hole)
		{
			register UInt32 child;
			register node tmp = pq->array[ hole - 1 ];

			/*for( ; hole * 2 <= pq->length ; hole = child )
			{
				child = hole * 2;
				if( child != pq->length && pq->array[ child ].pri < pq->array[ child - 1 ].pri )
					child++;
				if( pq->array[ child -1 ].pri < tmp.pri )
					pq->array[ hole - 1 ] = pq->array[ child - 1 ];
				else
					break;
			}*/

			register UInt32 _hole_2 = hole * 2;

			for( ; _hole_2 <= pq->length ; hole = child, _hole_2 = hole * 2)
			{
				child = _hole_2;
				if( child != pq->length && pq->array[ child ].pri < pq->array[ child - 1 ].pri )
					child++;
				if( pq->array[ child -1 ].pri < tmp.pri )
					pq->array[ hole - 1 ] = pq->array[ child - 1 ];
				else
					break;
			}


			pq->array[ hole - 1] = tmp;
		}
		//----------------------------------------------------
		// Removes the minimum element and returns it
		void *priq_pop(struct pri_queue *pq, ps2_priority *pri)
		{
#if defined __Delta_FIFO_Enabled__
			RingBuffer<void *> *fifo = pq->fifo;
			if(fifo->GetCount() != 0)
			{
				*pri = pq->cts;
				return fifo->Dequeue();
			}
#endif

			if(is_empty(pq))
				return null;

/*#if defined __Delta_FIFO_Enabled__
			pq->cts = pq->array[0].pri;
#endif*/

			*pri = pq->array[0].pri;
			void *data = pq->array[0].data;
			pq->array[ 0 ] = pq->array[pq->length-- - 1];
			percolate_down(pq, 1);

// If there are other items that have the same priority with the recent
// popped item, we must remove them from pq and move them into the fifo.
#if defined __Delta_FIFO_Enabled__
			//if(is_empty(pq))
			//	return null;

			while(!is_empty(pq))
			{
				// Check the top.
				if(pq->array[0].pri.timestamp == pri->timestamp)
				{
					pq->fifo->Enqueue(pq->array[0].data);

					pq->array[ 0 ] = pq->array[pq->length-- - 1];
					percolate_down(pq, 1);
				}
				else
					break;
			}

			pq->cts = *pri;

			/*if(pq->fifo->GetCount() != 0)
			{
				//pq->cts.timestamp = pri->timestamp;
				pq->cts = *pri;
			}*/
#endif

/*#if defined __Delta_Enabled1__
			std::cout << "pop pri (" << pri->timestamp << ", " << pri->delta << ") (timestamp, delta)" << endl;
#else
			std::cout << "pop pri (" << pri->timestamp << ") (timestamp)" << endl;
#endif

            EventInterface * e= (EventInterface *)data;
            std::cout << "sender(" << e->GetSenderReference() << "," << LogicalProcess::GetLogicalProcessById(e->GetSenderReference())->GetName().get_BaseStream() << ")";
            std::cout << " receiver(" << e->GetReceiverReference() << "," << LogicalProcess::GetLogicalProcessById(e->GetReceiverReference())->GetName().get_BaseStream() << ")";
            std::cout << " sendTime " << e->GetSendTime();
            std::cout << " receiveTime " << e->GetReceiveTime() << "\n---------------------" << std::endl;

			//printf("pop length: %d\n", pq->length);*/

			return data;
		}
		//----------------------------------------------------
		// Builds the heap if the array was populated manually
		void build_heap(struct pri_queue *pq)
		{
			register UInt32 i;
			for(i = pq->length / 2 ; i > 1 ; i-- )
				percolate_down(pq, i);
		}
		//----------------------------------------------------
		void priq_test()
		{
			struct pri_queue *pq = priq_new(128);

			priq_push(pq, 0, 3);
			priq_push(pq, 0, 3);
			priq_push(pq, 0, 4);
			priq_push(pq, 0, 1);
			priq_push(pq, 0, 1);
			priq_push(pq, 0, 4);
			priq_push(pq, 0, 4);
			priq_push(pq, 0, 4);
			priq_push(pq, 0, 5);

			printf("\n");
			printf("Heap Sorted:\n");
			while(!is_empty(pq))
			{
				ps2_priority pri;
				priq_pop(pq, &pri);
#if defined __Delta_Enabled1__
				std::cout << "pri (" << pri.timestamp << ", " << pri.delta << ") (timestamp, delta)" << endl;
#else
				std::cout << "pri (" << pri.timestamp << ") (timestamp)" << endl;
#endif
			}
			printf("\n");

			priq_delete(pq);
		}
		//----------------------------------------------------
	};
};
//**************************************************************************************************************//
