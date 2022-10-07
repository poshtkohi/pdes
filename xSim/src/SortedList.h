/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_Collections_SortedList_h__
#define __Parvicursor_xSim_Collections_SortedList_h__

#include <general.h>
#include <StaticFunctions/StaticFunctions.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>
#include <System/ObjectDisposedException/ObjectDisposedException.h>
#include <System/ArgumentException/ArgumentOutOfRangeException.h>

using namespace System;

//#include "State.h"
//using namespace Parvicursor::xSim;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		namespace Collections
		{
			//----------------------------------------------------
			template <class T>
			class SortedList : public Object
			{
				public: typedef void (*OnRemove)(T val);
				public: typedef void (*OnDispose)(T val);
				private: struct DoublyLinkedNode
				{
					struct DoublyLinkedNode *prev; // A reference to the previous node
					struct DoublyLinkedNode *next; // A reference to the next node
					T val; // Data or a reference to data
					//Object *data;
					UInt64 pri; // Priority for the node
					//Int32 delta; // Delta for simultaneous events
				};
				/*public: struct ListItem
				{
					DoublyLinkedNode *node;
				};
				/*---------------------fields-----------------*/
				private: Int32 capacity;
                private: Int32 count;
				private: DoublyLinkedNode *firstNode;   // points to first node of list
				private: DoublyLinkedNode *lastNode;    // points to last node of list
				private: OnDispose onDispose;
				private: bool disposed;
				/*---------------------methods----------------*/
				// SortedList Class constructor.
				public: SortedList(OnDispose onDispose)
				{
					//if(callback == null)
					//	throw ArgumentNullException("callback");

					this->onDispose = onDispose;
					capacity = 0;
					count = 0;
					firstNode = lastNode = null;
					disposed = false;
				}
				//----------------------------------------------------
				// SortedList Class destructor.
				public: ~SortedList()
				{
					//cout << "~SortedList()" << endl;
					if(!disposed)
					{
						Clear();
						disposed = true;
					}
				}
				//----------------------------------------------------
				private: void Clear()
				{
					if(disposed)
						throw ObjectDisposedException("SortedList", "The SortedList has been disposed");

					if(count == 0)
						return ;

					//cout << "Clear()" << endl;

					DoublyLinkedNode *iter = firstNode;
					DoublyLinkedNode *next = null;

					while(iter != null)
					{
						next = iter->next;
						if(onDispose != null)
							onDispose(iter->val);
						delete iter; /// to free list
						count--;
						iter = next;
					}
				}
				//----------------------------------------------------
				// Adds a new data item to the SortedList.
				public: void Add(T val, UInt64 pri)
				{
					DoublyLinkedNode *newNode = new DoublyLinkedNode(); /// from free list
					newNode->val = val;
					newNode->pri = pri;

					/*if(firstNode == null)
					{
						newNode->next = null;
						newNode->prev = null;
						firstNode = lastNode = newNode;
					}
					else
						InsertAfter(firstNode, newNode);*/
					//InsertEnd(newNode);

					//cout << "PE: " << PeID << " firstNode: " << firstNode << " lastNode: " << lastNode << " pri: " << pri << endl;
					//cout << "firstNode: " << firstNode << " lastNode: " << lastNode << " pri: " << pri << endl;

					if(firstNode == null)
						InsertBeginning(newNode);
					else if(pri <= firstNode->pri) //
						InsertBeginning(newNode);
					else if(pri >= lastNode->pri) //
						InsertEnd(newNode);
					// now must have at least two items in list
					else
					{
						//DoublyLinkedNode *oldIter = firstNode;
						//DoublyLinkedNode *iter = firstNode->next;
						//DoublyLinkedNode *oldIter = lastNode;
						DoublyLinkedNode *iter = lastNode->prev;
						while(iter->pri < pri) {
							//oldIter = iter;
							iter = iter->prev;
							//iter = iter->next;
						}
						// here iter.data >= x
						// insert x into a new node before iter
						InsertBefore(iter, newNode);
					}

					count++;
				}
				//----------------------------------------------------
				// Removes all elements with the value greater than or equal to pri
				// from the sorted list. The callback onRemove is called when removing
				// nodes passing the data. The last item prior to the pri is filled into
				// the lastItem structure. If there is no such item, lastItem.node will be filled
				// with null. The search to remove the items are done from the end of the sorted list.
				// Implement here a free list.
				public: void RemoveFromBackward(UInt64 pri, OnRemove onRemove)
				{

					//TraverseForward();

					if(firstNode == null)
					{
						//lastItem->node = null;
						//lastItem->data = null;
						//lastItem->pri = -1;

						return ;
					}
					if(pri < firstNode->pri || pri > lastNode->pri)
					{
						//lastItem->node = null;
						//lastItem->data = null;
						//lastItem->pri = -1;
						return ;
					}

					//DoublyLinkedNode *oldIter = firstNode;
					// Searches from start.
					/*DoublyLinkedNode *iter = firstNode;
					DoublyLinkedNode *next = null;

					while(iter != null && iter->pri < pri)
					{
						//oldIter = iter;
						iter = iter->next;
					}*/

					// Searches from end.
					/*DoublyLinkedNode *iter = lastNode;
					DoublyLinkedNode *next = null;

					while(iter != null && iter->pri > pri)
					{
						//oldIter = iter;
						iter = iter->prev;
					}*/

					// Searches from end.
					DoublyLinkedNode *iter = lastNode;
					DoublyLinkedNode *prev = null;

					while(iter != null && pri <= iter->pri)
					{
						if(onRemove != null)
							onRemove(iter->val);

						count--;
						prev = iter->prev;
						delete iter; /// to free list
						iter = prev;
					}
					lastNode = iter;
					if(lastNode != null)
						lastNode->next = null;
					return;

					/*if(iter == firstNode && iter == lastNode)
					{
						if(onRemove != null)
							onRemove(iter->val);
						delete iter; /// to free list
						firstNode = lastNode = null;
						//lastItem->data = null;
						//lastItem->pri = -1;
						//lastItem->node = null;
						count--;
						return ;
					}
					else
					{
						if(iter == firstNode)
						{
							firstNode = lastNode = null;
							//lastItem->data = null;
							//lastItem->pri = -1;
							//lastItem->node = null;
						}
						else
						{
							lastNode = iter->prev;
							lastNode->next = null;
							//lastItem->data = lastNode->data;
							//lastItem->pri = lastNode->pri;
							//lastItem->node = lastNode;
						}

						while(iter != null)
						{
							next = iter->next;
							if(onRemove != null)
								onRemove(iter->val);
							delete iter; /// to free list
							count--;
							iter = next;
						}
					}*/
				}
				//----------------------------------------------------
				// Removes all elements with the value greater than or equal to pri
				// from the sorted list. The callback onRemove is called when removing
				// nodes passing the data. The last item prior to the pri is filled into
				// the lastItem structure. If there is no such item, lastItem.node will be filled
				// with null. The search to remove the items are done from the start of the sorted list.
				// Implement here a free list.
				public: void RemoveFromForward(UInt64 pri, OnRemove onRemove)
				{

					//TraverseForward();

					if(firstNode == null)
					{
						//lastItem->node = null;
						//lastItem->data = null;
						//lastItem->pri = -1;

						return ;
					}
					if(pri < firstNode->pri || pri > lastNode->pri)
					{
						//lastItem->node = null;
						//lastItem->data = null;
						//lastItem->pri = -1;
						return ;
					}

					//DoublyLinkedNode *oldIter = firstNode;
					// Searches from start.
					DoublyLinkedNode *iter = firstNode;
					DoublyLinkedNode *next = null;

					while(iter != null && iter->pri < pri)
					{
						//oldIter = iter;
						iter = iter->next;
					}

					if(iter == firstNode && iter == lastNode)
					{
						if(onRemove != null)
							onRemove(iter->val);
						delete iter; /// to free list
						firstNode = lastNode = null;
						//lastItem->data = null;
						//lastItem->pri = -1;
						//lastItem->node = null;
						count--;
						return ;
					}
					else
					{
						if(iter == firstNode)
						{
							firstNode = lastNode = null;
							//lastItem->data = null;
							//lastItem->pri = -1;
							//lastItem->node = null;
						}
						else
						{
							lastNode = iter->prev;
							lastNode->next = null;
							//lastItem->data = lastNode->data;
							//lastItem->pri = lastNode->pri;
							//lastItem->node = lastNode;
						}

						while(iter != null)
						{
							next = iter->next;
							if(onRemove != null)
								onRemove(iter->val);
							delete iter; /// to free list
							count--;
							iter = next;
						}
					}
				}
				//----------------------------------------------------
				// This methods first finds the search location in the list.
				// Then all elements from the location of search are removed.
				// The callback onRemove is called when removing
				// nodes passing the data. The last item prior to the pri is filled into
				// the lastItem structure. If there is no such item, lastItem.node will be filled
				// with null. The search to remove the items are done from the start of the sorted list.
				// Implement here a free list.
				public: bool RemoveFromSearchForward(T search, OnRemove onRemove)
				{

					//TraverseForward();

					bool found = false;

					if(firstNode == null)
                       	return found;

					//DoublyLinkedNode *oldIter = firstNode;
					// Searches from start.
					DoublyLinkedNode *iter = firstNode;
					DoublyLinkedNode *next = null;

					while(iter != null)
					{
						if(iter->val == search)
						{
							found = true;
							break;
						}
						iter = iter->next;
					}

					if(!found)
						return found;

					if(iter == firstNode && iter == lastNode)
					{
						if(onRemove != null)
							onRemove(iter->val);
						delete iter; /// to free list
						firstNode = lastNode = null;
						//lastItem->data = null;
						//lastItem->pri = -1;
						//lastItem->node = null;
						count--;
					}
					else
					{
						if(iter == firstNode)
						{
							firstNode = lastNode = null;
							//lastItem->data = null;
							//lastItem->pri = -1;
							//lastItem->node = null;
						}
						else
						{
							lastNode = iter->prev;
							lastNode->next = null;
							//lastItem->data = lastNode->data;
							//lastItem->pri = lastNode->pri;
							//lastItem->node = lastNode;
						}

						while(iter != null)
						{
							next = iter->next;
							if(onRemove != null)
								onRemove(iter->val);
							delete iter; /// to free list
							count--;
							iter = next;
						}
					}

					return found;
				}
				//----------------------------------------------------
				// Traversal of a doubly linked list can be in either direction.
				// In fact, the direction of traversal can change many times, if desired.
				// Traversal is often called iteration, but that choice of terminology is
				// unfortunate, for iteration has well-defined semantics (e.g., in mathematics)
				// which are not analogous to traversal.
				public: void TraverseForward()
				{
					cout << "Traversing forward...\n";
					DoublyLinkedNode  *node = firstNode;
					while(node != null)
					{
						cout << "\t Priority " << node->pri << " Val: " << node->val << endl;
						node = node->next;
					}
				}
				//----------------------------------------------------
				public: void TraverseBackward()
				{
					cout << "Traversing backward...\n";
					DoublyLinkedNode  *node = lastNode;
					while(node != null)
					{
						cout << "\t Priority " << node->pri << " Val: " << node->val << endl;
						node = node->prev;
					}
				}
				//----------------------------------------------------
				// Inserts a node after a given node.
				private: inline void InsertAfter(DoublyLinkedNode *node, DoublyLinkedNode *newNode)
				{
					newNode->prev = node;
					newNode->next = node->next;
					if(node->next == null)
						lastNode = newNode;
					else
						node->next->prev = newNode;
					node->next = newNode;
				}
				//----------------------------------------------------
				// Inserts a node before a given node.
				private: inline void InsertBefore(DoublyLinkedNode *node, DoublyLinkedNode *newNode)
				{
					newNode->prev = node->prev;
					newNode->next = node;
					if(node->prev == null)
						firstNode = newNode;
					else
						node->prev->next = newNode;
					node->prev = newNode;
				}
				//----------------------------------------------------
				// Insert a node at the beginning of a possibly empty list.
				private: inline void InsertBeginning(DoublyLinkedNode *newNode)
				{
					if(firstNode == null)
					{
						firstNode = newNode;
						lastNode = newNode;
						newNode->prev = null;
						newNode->next = null;
					}
					else
						InsertBefore(firstNode, newNode);
				}
				//----------------------------------------------------
				// Inserts a node at the end of a possibly empty list.
				private: inline void InsertEnd(DoublyLinkedNode *newNode)
				{
					if(lastNode == null)
						InsertBeginning(newNode);
					else
						InsertAfter(lastNode, newNode);
				}
				//----------------------------------------------------
				// Logically removes a node from the list. Deletion of the node must be
				// performed by the caller. Removal of a node is easier than insertion, but
				// requires special handling if the node to be removed is the firstNode or lastNode.
				private: inline void LogicalRemove(DoublyLinkedNode *node)
				{
					if(node->prev == null)
						firstNode = node->next;
					else
						node->prev->next = node->next;
					if(node->next == null)
						lastNode = node->prev;
					else
						node->next->prev = node->prev;
				}
				//----------------------------------------------------
				public: inline Int32 Size()
				{
					return count;
				}
				/*--------------------------------------------*/
			};
			//----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

#endif


/*
// Sample Usage
void _OnRemove(void *data)
{
	cout << "Hello World! " << (Int32)data << endl;
	return ;
}

int main(int argc, char **argv)
{
	SortedList<Int32> ol = SortedList<Int32>();

	for(register Int32 i = 0 ; i < 100 ; i++)
	ol.Add(i, 99 - i);
	//ol.Add(i, i);

	ol.RemoveFrom(0, _OnRemove);
	ol.TraverseForward();
	//ol.TraverseBackward();
	return 0;
}
*/

