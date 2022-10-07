/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __Parvicursor_xSim_PriorityQueueInterface_h__
#define __Parvicursor_xSim_PriorityQueueInterface_h__

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>

using namespace System;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		// the Callback definition.
		typedef void (*QueueStateNotificationCallback)(Object *, Int32);
		//----------------------------------------------------
		interface class PriorityQueueInterface : public Object
		{
			/*---------------------methods----------------*/
            // Enters an item in the queue. The callback is invoked when the queue is full, the state object is passed to the callback.
            public: virtual bool Enqueue(Object *obj, Int64 timestamp) = 0;
            // Removes an item from the queue. Returns null if queue is empty. The callbackBefore is invoked when the queue is empty.
			// The callbackAfter is called when it's believed that the queue may not be empty after a followed blocking due to an empty queue.
			// The state object is passed to both callbacks.
            public: virtual Object *Dequeue(Out Int64 &timestamp, QueueStateNotificationCallback OnBlocking, QueueStateNotificationCallback OnUnblocking, Object *state, Int32 OwnerClusterID) = 0;
			// Returns the object at the beginning of the Queue without removing it. The callbackBefore is invoked when the queue is empty.
			// The callbackAfter is called when it's believed that the queue may not be empty after a followed blocking due to an empty queue.
			// The state object is passed to both callbacks.
			// OwnerClusterID specifies the ID of the cluster that is associated with this queue instance.
            public: virtual Object *Peek(Out Int64 &timestamp, QueueStateNotificationCallback OnBlocking, QueueStateNotificationCallback OnUnblocking, Object *state, Int32 OwnerClusterID) = 0;
		};
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

