/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_Collections_List_h__
#define __Parvicursor_xSim_Collections_List_h__

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
			class List : public Object
			{
				public: typedef void (*OnRemove)(T val);
				private: struct SinglyLinkedNode
				{
					SinglyLinkedNode *next; // A reference to the next node
					T val; // Data or a reference to data
				};
				/*---------------------fields-----------------*/
				private: UInt32 capacity;
                private: UInt32 count;
				private: SinglyLinkedNode *firstNode;   // points to first node of list
				private: SinglyLinkedNode *lastNode;   // points to last node of list
				private: OnRemove onRemove;
				private: bool disposed;
				/*---------------------methods----------------*/
				// SortedList Class constructor.
				public: List(OnRemove onRemove)
				{
					//if(callback == null)
					//	throw ArgumentNullException("callback");

					this->onRemove = onRemove;
					capacity = 0;
					count = 0;
					firstNode = lastNode = null;
					disposed = false;
				}
				//----------------------------------------------------
				// SortedList Class destructor.
				public: ~List()
				{
					//cout << "~SortedList()" << endl;
					if(!disposed)
					{
						Clear();
						disposed = true;
					}
				}
				//----------------------------------------------------
				public: void Clear()
				{
					//if(disposed)
					//	throw ObjectDisposedException("List", "The List has been disposed");

					if(count == 0)
						return ;

					SinglyLinkedNode *curPtr = firstNode;
					SinglyLinkedNode *temp ;
					while(curPtr != null)
					{
						temp = curPtr;
						curPtr = curPtr->next;
						if(onRemove != null)
							onRemove(temp->val);
						delete temp;
					}
					firstNode = lastNode = null;
					count = 0;
				}
				//----------------------------------------------------
				// Adds a new data item to the list.
				public: void Add(T val)
				{
					//if(disposed)
					//	throw ObjectDisposedException("List", "The List has been disposed");

					SinglyLinkedNode *newPtr = new SinglyLinkedNode();
					if(newPtr != null)
					{
						newPtr->val = val;
						newPtr->next = null;

						if(firstNode == null)
							firstNode = lastNode = newPtr;
						else
						{
							lastNode->next = newPtr;
							lastNode = newPtr;
						}
						count++;
						return ;
					}
					else
					{
						delete newPtr;
						return ;
					}
					count++;
				}
				//----------------------------------------------------
				// Removes an item from list. True will be returned if
				// the item is found.
				public: bool Remove(T val)
				{
					//if(disposed)
					//	throw ObjectDisposedException("List", "The List has been disposed");

					SinglyLinkedNode *curPtr = firstNode;
					SinglyLinkedNode *nextPtr = lastNode;
					bool found = false;
					while(nextPtr != null)
					{
						if(val == nextPtr->val)
						{
							count--;
							found = true;
							if(onRemove != null)
								onRemove(nextPtr->val);
							if(nextPtr == firstNode)
							{
								firstNode = firstNode->next;
								delete nextPtr;
								break;
							}
							else
							{
								if(nextPtr == lastNode)
									lastNode = curPtr;

								curPtr->next = nextPtr->next;
								delete nextPtr;
								break;
							}
						}
						else
						{
							curPtr = nextPtr;
							nextPtr = nextPtr->next;
						}
					}

					return found;
				}
				//----------------------------------------------------
				// Traversal of a singly linked list.
				public: void Traverse()
				{
					cout << "Traversing ...\n";
					SinglyLinkedNode  *node = firstNode;
					while(node != null)
					{
						cout << "\t Val: " << node->val << endl;
						node = node->next;
					}
				}
				//----------------------------------------------------
				public: inline Int32 GetCount()
				{
					//if(disposed)
					//	throw ObjectDisposedException("List", "The List has been disposed");

					return count;
				}
				//----------------------------------------------------
				//----------------------------------------------------
				/*--------------------------------------------*/
			};
			//----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif
