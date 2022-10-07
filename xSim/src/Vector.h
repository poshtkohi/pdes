/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_psc_Vector_h__
#define __Parvicursor_xSim_psc_Vector_h__

#include <general.h>
#include <StaticFunctions/StaticFunctions.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>

using namespace System;

#include <string.h>

/* Compute the nearest power of 2 number that is
* less than or equal to the value passed in.
*/

/*static Int32 nearestPower( Int32 value )
{
	int i = 1;
	if (value == 0) return -1;      // Error!
	for (;;) {
		if (value == 1) return i;
		else if (value == 3) return i*4;
		value >>= 1; i *= 2;
	}
}*/

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		namespace psc
		{
			//----------------------------------------------------
			template <class T>
			class Vector : public Object
			{
				//typedef void (*OnRemove)(const T &val);
				/*---------------------fields-----------------*/
				private: UInt32 capacity_;
                private: UInt32 count_;
                private: T *arr_;
				/*---------------------methods----------------*/
				// Vector Class constructor.
				public: Vector(UInt32 capacity = 1)
				{
					if(capacity <= 0)
					    throw ArgumentOutOfRangeException("capacity", "capacity must be greater than zero");

					capacity_ = capacity;
					count_ = 0;
					//arr_ = (T *)xParvicursor_memalign(xParvicursor_getpagesize(), sizeof(T) * capacity_);
					arr_ = (T *)::malloc(sizeof(T) * capacity_);
				}
                // Vector Class Copy constructor.
				/*public: Vector(Vector &v)
				{
                    Vector(v.Size());
                    memccpy(arr_, v.arr_, sizeof(T) * v.Size());
				}*/
				//----------------------------------------------------
				/*public: Vector()
				{
					capacity_ = 0;
					count_ = 0;
					arr_ = null;
				}*/
				//----------------------------------------------------
				// Vector Class destructor.
				public: ~Vector()
				{
					//cout << "~Vector() count " << count_ << " arr_ " << arr_ << endl;
					if(arr_ != null)
						::free(arr_);
					//xParvicursor_free(arr_);
				}
				//----------------------------------------------------
				public: inline UInt32 Capacity()
				{
					return capacity_;
				}
				//----------------------------------------------------
				public: inline UInt32 Size()
				{
					return count_;
				}
				//----------------------------------------------------
				public: inline void PushBack(const T &val)
				{
					//cout << "capacity_ " << capacity_ << " count_ " << count_ << " val " << val << endl;
					//if(capacity_ == count_)
					//    throw ArgumentOutOfRangeException("Vector", "The vector is full with the size of its intial capacity");
					/*if(capacity_ == 0)
					{
						capacity_ = 1;
						//arr_ = (T *)xParvicursor_memalign(xParvicursor_getpagesize(), sizeof(T) * capacity_);
						arr_ = (T *)::malloc(sizeof(T) * capacity_);
					}*/
					if(count_ == capacity_)
					{
						capacity_ = 2 * count_;
						//T *temp = (T *)xParvicursor_memalign(xParvicursor_getpagesize(), sizeof(T) * capacity_);
						T *temp = (T *)malloc(sizeof(T) * capacity_);
						::memcpy(temp, arr_, count_ * sizeof(T) );
						//xParvicursor_free(arr_);
						::free(arr_);
						arr_ = temp;
					}

					arr_[count_] = val;
					count_++;
				}
				//----------------------------------------------------
				public: inline void UnsafePushBack(const T &val)
				{
					arr_[count_] = val;
					count_++;
				}
				//----------------------------------------------------
                // Read-only array-style access.
                public: inline const T &operator[](Int32 n) const
				{
					//if(n >= count_)
					//    throw ArgumentOutOfRangeException("n", "n is greater than Size()");

					return arr_[n];
				}
				//----------------------------------------------------
                public: inline T &operator[](Int32 n)
				{
					//if(n >= count_)
					//    throw ArgumentOutOfRangeException("n", "n is greater than Size()");

					return arr_[n];
				}
				//----------------------------------------------------
				public: inline void Reserve(Int32 n)
				{
					//if(n >= count_)
					//    throw ArgumentOutOfRangeException("n", "n is greater than Size()");

					if(arr_ != null)
						free(arr_);
					capacity_ = n;
					count_ = 0;
					//arr_ = (T *)xParvicursor_memalign(xParvicursor_getpagesize(), sizeof(T) * capacity_);
					arr_ = (T *)::malloc(sizeof(T) * capacity_);
				}
				//----------------------------------------------------
				public: inline void Reset()
				{
					count_ = 0;
				}
				//----------------------------------------------------
				public: inline void RemoveAt(UInt32 n)
				{
                    if(n == 0 && capacity_ == 1 && count_ == 1)
                    {
                        count_--;
                        return;
                    }

					for(register UInt32 i = n ; i < count_ - 1 ; i++)
						arr_[i] = arr_[i + 1];

					count_--;
				}
				//----------------------------------------------------
				public: inline T *GetBaseMemory()
				{
					return arr_;
				}
				/*--------------------------------------------*/
			};
			//----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif

