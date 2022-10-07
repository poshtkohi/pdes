/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "priority_queue_lc.h"
#include "EventInterface.h"
#include "LogicalProcess.h"

using namespace Parvicursor::xSim;

//**************************************************************************************************************//
namespace Parvicursor
{
	namespace PS2
	{
		//----------------------------------------------------
		struct pri_queue_lc *priq_new_lc(UInt32 size)
		{
			if(size == 0)
				size = 128;
			pri_queue_lc *pq = (pri_queue_lc *)::malloc(sizeof(pri_queue_lc));
			pq->array = (node_lc *)::malloc(sizeof(node_lc) * size);
			pq->alloc = size;
			pq->length = 0;
			return pq;
		}
		//----------------------------------------------------
		void priq_delete_lc(struct pri_queue_lc *pq)
		{
			::free(pq->array);
			::free(pq);
		}
		//----------------------------------------------------
		// Percolates the hole from the new position and fills the pri once no elements above are less
		void priq_push_lc(struct pri_queue_lc *pq, In void *data, const PdesTime &pri)
		{
            if(pq->length >= pq->alloc)
			{
				pq->alloc *= 2;
				pq->array = (node_lc *)realloc(pq->array, sizeof(node_lc) * pq->alloc);
				pq->allocations++;
			}

			register UInt32 hole = ++pq->length;

			register UInt32 _hole_2 = hole/2 - 1;\
			for( ; hole > 1 && pri < pq->array[_hole_2].pri ; hole /= 2, _hole_2 = hole/2 - 1)\
				pq->array[ hole - 1] = pq->array[ _hole_2];

			pq->array[ hole - 1 ].pri = pri;
			pq->array[ hole - 1 ].data = data;

            //if(pq->cluster == 1)
            //    std::cout << "cluster " << pq->cluster << " " << pri << std::endl;
		}
		//----------------------------------------------------
		// Percolates the hole down after removing the top element
		void percolate_down(struct pri_queue_lc *pq, UInt32 hole)
		{
			register UInt32 child;
			register node_lc tmp = pq->array[ hole - 1 ];

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
		void *priq_pop_lc(struct pri_queue_lc *pq, Out PdesTime *pri)
		{
			if(is_empty_lc(pq))
				return null;

			*pri = pq->array[0].pri;
			void *data = pq->array[0].data;
			pq->array[ 0 ] = pq->array[pq->length-- - 1];
			percolate_down(pq, 1);

            //if(pq->cluster == 1)
            //    std::cout << "cluster " << pq->cluster << " " << *pri << std::endl;

			return data;
		}
		//----------------------------------------------------
		// Builds the heap if the array was populated manually
		void build_heap(struct pri_queue_lc *pq)
		{
			register UInt32 i;
			for(i = pq->length / 2 ; i > 1 ; i-- )
				percolate_down(pq, i);
		}
		//----------------------------------------------------
		void priq_test_lc()
		{
			struct pri_queue_lc *pq = priq_new_lc(128);

			/*priq_push_lc(pq, 0, PdesTime(0, 1, 2));
			priq_push_lc(pq, 0, PdesTime(1, 2, 1));
			priq_push_lc(pq, 0, PdesTime(0, 1, 1));
			priq_push_lc(pq, 0, PdesTime(2, 1, 4));
			priq_push_lc(pq, 0, PdesTime(3, 1, 4));
			priq_push_lc(pq, 0, PdesTime(4, 10, 1));
			priq_push_lc(pq, 0, PdesTime(4, 1, 1));
			priq_push_lc(pq, 0, PdesTime(5, 0, 10));
			priq_push_lc(pq, 0, PdesTime(5, 0, 11));*/

			printf("\n");
			printf("Heap Sorted:\n");
			while(!is_empty_lc(pq))
			{
				PdesTime pri;
				priq_pop_lc(pq, &pri);
				std::cout << pri << std::endl;
			}
			printf("\n");

			priq_delete_lc(pq);
		}
		//----------------------------------------------------
	};
};
//**************************************************************************************************************//
