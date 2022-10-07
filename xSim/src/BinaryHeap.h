/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_Collections_BinaryHeap_h__
#define __Parvicursor_xSim_Collections_BinaryHeap_h__

#include <general.h>
#include <StaticFunctions/StaticFunctions.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
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
			class BinaryHeap : public Object
			{
				typedef void (*OnRemove)(T val);
				//typedef void (*OnDispose)(T val);
				/*---------------------fields-----------------*/
				private: struct Element { T val; UInt64 pri; };
				private: Int32 capacity_;
                private: Int32 count_;
                private: Element *arr_;
				private: OnRemove onRemove;
				//private: OnDispose onDispose;
				/*---------------------methods----------------*/
				// BinaryHeap Class constructor.
				public: BinaryHeap(Int32 capacity, OnRemove onRemove/*, OnDispose onDispose*/)
				{
					//if(capacity <= 0)
					//    throw ArgumentOutOfRangeException("capacity", "capacity must be greater than zero");

					if(capacity < 4)
						capacity = 4;
					capacity_ = capacity;//+ 1;
					count_ = 0;
					this->onRemove = onRemove;
					//this->onDispose = onDispose;
					arr_ = (Element *)malloc(sizeof(Element) * capacity_);
				}
				//----------------------------------------------------
				// BinaryHeap Class destructor.
				public: ~BinaryHeap()
				{
					if(arr_ != null)
					{
						/*if(onDispose != null)
							for(register  Int32 i = 1 ; i < count_ ; i++)
								onDispose(arr_[i].val);*/
						if(onRemove != null)
							for(register  Int32 i = 1 ; i < count_ ; i++)
								onRemove(arr_[i].val);

						free(arr_);
					}
				}
				//----------------------------------------------------
				public: inline Int32 Capacity()
				{
					return capacity_;
				}
				//----------------------------------------------------
				public: inline Int32 GetCount()
				{
					return count_;
				}
				//----------------------------------------------------
				public: inline void Enqueue(T val, UInt64 pri)
				{
					if(count_ + 1 >= capacity_)
					{
						capacity_ *= 2;
						arr_ = (Element *)realloc(arr_, sizeof(Element) * capacity_);
					}

					//cout << "BinaryHeap::Insert() val " << val << " pri " << pri << " count_ " << count_ << " capacity_ " << capacity_ << " arr_[count_ + 1] " << arr_[count_ + 1].pri << endl;
					//cout << "BinaryHeap::Insert() val " << val << " pri " << pri << " count_ " << count_ << " capacity_ " << capacity_ << " arr_ " << arr_ << endl;
					//printf("BinaryHeap::Insert() pri %d count_ %d capacity_ %d arr_ %p\n", pri, count_, capacity_, arr_);

					arr_[count_ + 1].val = val; // Insert val in the "fartest left location"
					arr_[count_ + 1].pri = pri;	 // This preserves the "complete" bin tree prop

					count_++;		      // We have 1 more node

					HeapFilterUp(count_); // Filter the inserted node up
											// This preserves the "min. value at root" prop
				}
				//----------------------------------------------------
				// HeapFilterUp(k): Filter the node a[k] to its proper position
				// in the heap
				private: void HeapFilterUp(Int32 k)
				{
					Int32 parent;                 /* parent = parent */
					Element help;

					while(k != 1)    /* k has a parent node */
					{ /* Parent is not the root */

						parent = k / 2;

						if(arr_[k].pri < arr_[parent].pri)
						{
							help = arr_[parent];
							arr_[parent] = arr_[k];
							arr_[k] = help;

							/* ===============================
							Continue filter up one level
							=============================== */
							k = parent;          // k moved up one level
						}
						else
						{
							break;
						}

					}
				}
				//----------------------------------------------------
				private: void HeapFilterDown(Int32 k)
				{
					Int32 child1, child2;
					Element help;


					while ( 2*k <= count_ )
					{
						child1 = 2*k;                 // Child1 = left  child of k
						child2 = 2*k+1;               // Child2 = right child of k

						if(child2 <= count_)
						{
							/* ========================================
							Node k has 2 children nodes....
							Find the min. of 3 nodes !!!
							======================================== */
							if( arr_[k].pri < arr_[child1].pri && arr_[k].pri < arr_[child2].pri )
							{
								/* -------------------------------------------------------
								Node k is in correct location... It's a heap. Stop...
								------------------------------------------------------- */
								break;
							}
							else
							{
								/* =========================================
								Replace a[k] with the smaller child node
								========================================= */
								if ( arr_[child1].pri < arr_[child2].pri )
								{
									/* -------------------------------------------------
									Child1 is smaller: swap a[k] with a[child1]
									------------------------------------------------- */
									help = arr_[k];
									arr_[k] = arr_[child1];
									arr_[child1] = help;

									k = child1;         // Replacement node is now a[child1]
								}
								else
								{
									/* -------------------------------------------------
									Child2 is smaller: swap a[k] with a[child2]
									------------------------------------------------- */
									help = arr_[k];
									arr_[k] = arr_[child2];
									arr_[child2] = help;

									k = child2;        // Replacement node is now a[child2]
								}
							}
						}
						else
						{
							/* ========================================
							Node k only has a left child node
							======================================== */
							if ( arr_[k].pri < arr_[child1].pri )
							{
								/* -------------------------------------------------------
								Node k is in correct location... It's a heap. Stop...
								------------------------------------------------------- */
								break;
							}
							else
							{
								/* -------------------------------------------------------
								Child1 is smaller: swap a[k] with a[child1]
								------------------------------------------------------- */
								help = arr_[k];
								arr_[k] = arr_[child1];
								arr_[child1] = help;

								k = child1;         // Replacement node is now a[child1]
							}
						}
					}
				}
				//----------------------------------------------------
				private: Element remove(Int32 k)
				{
					Int32    parent;
					Element r;             // Variable to hold deleted value

					r = arr_[k];             // Save return value

					arr_[k] = arr_[count_];     // Replace deleted node with the right most leaf
					// This fixes the "complete bin. tree" property
					count_--;

					parent = k/2;

					if (k == 1 /* k is root */ || arr_[parent].pri < arr_[k].pri )
					{
						//System.out.println("\nHeap before filter DOWN:");
						//printHeap();
						HeapFilterDown(k);  // Move the node a[k] DOWN the tree
					}
					else
					{
						//System.out.println("\nHeap before filter UP:");
						//printHeap();
						HeapFilterUp(k);    // Move the node a[k] UP the tree
					}

					return r;
				}
				//----------------------------------------------------
				// Removes an item based on search. If an item is found,
				// rm will be called and the method returns true.
				public: bool Remove(T search)
				{

					bool found = false;

					//for(register Int32 i = 0 ; i < count_ ; i++)
					// A fast loop. arr_[0] is omitted.
					for(register Int32 i = count_ - 1 ; i != 0 ; i--)
					{
						if(arr_[i].val == search)
						{
							if(onRemove != null)
								onRemove(arr_[i].val);
							found = true;
							remove(i);
							break;
						}
					}

					return found;
				}
				//----------------------------------------------------
				// Prints the whole items.
				public: void Print()
				{
					for(register Int32 i = 1 ; i < count_ ; i++)
						cout << "Item" << i << ": " << arr_[i].val << endl;
				}
				//----------------------------------------------------
				// DeleteMin
				public: inline T Dequeue(Out UInt64 &pri)
				{
					Element elem = remove(1);
					pri = elem.pri;
					return elem.val;
				}
				//----------------------------------------------------
				// Peek min
				public: inline T Peek(Out UInt64 &pri)
				{
					pri = arr_[1].pri;
					return arr_[1].val;
				}
				//----------------------------------------------------
                // Read-only array-style access.
                public: inline const T &operator[](UInt32 n) const
				{
					//if(n >= count_)
					//    throw ArgumentOutOfRangeException("n", "n is greater than Size()");

					return arr_[n];
				}
				//----------------------------------------------------
                public: inline T &operator[](UInt32 n)
				{
					//if(n >= count_)
					//    throw ArgumentOutOfRangeException("n", "n is greater than Size()");

					return arr_[n];
				}
				//----------------------------------------------------
				private: void printnode(Int32 n, Int32  h)
				{
					for(register Int32 i = 0 ; i < h ; i++)
						cout << "        ";

					cout << "[" << arr_[n].pri << "]" << endl;
				}
				//----------------------------------------------------
				public: void PrintHeap()
				{
					if(count_ == 0)
					{
						cout << "*** heap is empty" << endl;
						cout << "================================" << endl;
						return;
					}

					ShowR(1, 0);
					cout << "================================" << endl;
				}
				//----------------------------------------------------
				private: void ShowR(Int32 n, Int32 h)
				{
					if (n > count_)
						return;

					ShowR(2*n+1, h+1);
					printnode(n, h);
					ShowR(2*n, h+1);
				}
				/*--------------------------------------------*/
			};
			//----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif


// Sample
/*
void __OnRemove(const Int32 &val)
{
	cout << "The val " << val << " was removed" << endl;
}

int main(int argc, char **argv)
{

	BinaryHeap<Int32>heap = BinaryHeap<Int32>(10);
	for(register Int32 i = 0 ; i < 100 ; i++)
		heap.Enqueue(i, 99 - i);
	//heap.PrintHeap();

	heap.Remove(1, __OnRemove);
	UInt64 pri;
	Int32 val = heap.Dequeue(pri);
	cout << "\nDeleteMin " << pri << endl;
	heap.Print();
	//cout << "Size(): " << heap.Size() << endl;
	return 0;
}
*/
