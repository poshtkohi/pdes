/**
    #define meta ...
    printf("%s\n", meta);
**/

/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_Collections_RingBuffer_h__
#define __Parvicursor_xSim_Collections_RingBuffer_h__

#include <general.h>
#include <StaticFunctions/StaticFunctions.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/Convert/Convert.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>

using namespace System;
//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		namespace Collections
		{
			//----------------------------------------------------
			template <class T>
			class RingBuffer: public Object
			{
				/*---------------------fields----------------*/
				// volatile is only used to keep compiler from placing values in registers.
				private: /*volatile*/ Int32 head; // UInt32 is to optimize n%DeltaQueueCapacity
				private: /*volatile*/ Int32 tail;
				//private: LockFreeCounter count;
				private: UInt32 count;
				private: UInt32 capacity;
				private: T *elements;
				/*---------------------methods----------------*/
				// RingBuffer Class constructor.
				public: RingBuffer(UInt32 capacity = 128)
				{
					if(capacity <= 0)
						throw ArgumentOutOfRangeException("capacity", "capacity is less than or equal to zero.");

					this->capacity = capacity;
					elements = (T *)::malloc(capacity * sizeof(T));

					head = tail = -1;
					//count = LockFreeCounter();
					count = 0;
				}
				//----------------------------------------------------
				// RingBuffer Class destructor.
				public: ~RingBuffer()
				{
					//if(!disposed)
					{
						::free(elements);
						//disposed = true;
					}
				}
				//----------------------------------------------------
				// Removes all items from the queue
				public: inline void Clear()
				{
					head = -1;
					tail = -1;
					//count = LockFreeCounter(0);
				}
				//----------------------------------------------------
				// Enters an item in the queue
				public: inline void Enqueue(T val/*, UInt64 timestamp*/)
				{
					if(count == capacity)
					{
						UInt32 capacity_ = capacity;
						capacity *= 2;
						T *elements_ = (T *)::malloc(capacity * sizeof(T));
						Int32 head_ = head;
						for(register UInt32 i = 0 ; i < capacity_ ; i++)
						{
							head_++;
							register UInt32 index1 = head_ % capacity_;
							register UInt32 index2 = head_ % capacity;
							elements_[index2] = elements[index1];
							//elements_[index2]->obj = elements[index1]->obj;
							//elements_[index2]->timestamp = elements[index1]->timestamp;
						}
						//for(register UInt32 i = 0 ; i < capacity_ ; i++)
						//	::free(elements[i]);
						::free(elements);
						elements = elements_;
					}
					// Increments tail index
					tail++;
					register UInt32 index = tail % capacity;
					elements[index] = val;
					count++;
				}
				//----------------------------------------------------
				// Removes an item from the queue. Returns null if queue is empty.
				public: inline T Dequeue(/*Out UInt64 &timestamp*/)
				{
					//if(count == 0)
					//	return null;

					head++;
					register UInt32 index = head % capacity;
					T val = elements[index];
					count--;
					return val;
				}
				//----------------------------------------------------
				public: inline T Peek(/*Out UInt64 &timestamp*/)
				{
					// The buffer is empty.
					//if(count.get_Count() == 0)
					//	return null;
					//if(count == 0)
					//	return null;
					register UInt32 index = (head + 1) % capacity;
					//Object *obj = elements[index]->obj;
					//timestamp = elements[index]->timestamp;
					T val = elements[index];
					return val;
				}
				//----------------------------------------------------
				public: inline UInt32 GetCount()
				{
					return count;
				}
				//private: inline Int32 GetCountInternal() { return count + count_deltas; }
				//----------------------------------------------------
			};
		}
	}
};
//**************************************************************************************************************//

#endif

