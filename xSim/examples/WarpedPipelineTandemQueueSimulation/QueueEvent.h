/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __QueueEvent__
#define __QueueEvent__

#include <DefaultEvent.h>
#include <cstdlib>
#include <SerializedInstance.h>
#include <ObjectID.h>
#include <FlatState.h>

class QueueEvent : public DefaultEvent
{
    //----------------------------------------------------
    private: const string myOwner;
    public: int frame;
    //public: int row;
    //----------------------------------------------------
    /// Obligatory methods to be implemented.

    // Constructor called by application.
    public: QueueEvent(const VTime &initSendTime,
              const VTime &initRecvTime,
              SimulationObject *initSender,
              SimulationObject *initReceiver);

    // Constructor called by deserializer.
    private: QueueEvent( const VTime &initSendTime,
             const VTime &initRecvTime,
             const ObjectID &initSender,
             const ObjectID &initReceiver,
             const unsigned int eventIdVal);


    public: ~QueueEvent(){}

    public: const string &getDataType() const
    {
        return getQueueEventDataType();
    }

    public: static Serializable *deserialize( SerializedInstance *instance );
    public: void serialize( SerializedInstance * ) const;
    public: bool eventCompare(const Event* event);
    //----------------------------------------------------
    // Optional methods to be implemented.
    const string &getOwner() const
    {
        return myOwner;
    }

    static const string &getQueueEventDataType()
    {
        static string QueueEventDataType = "QueueEvent";
        return QueueEventDataType;
    }

    public: unsigned int getEventSize() const
    {
        return sizeof(QueueEvent);
    }
    //----------------------------------------------------
};

#endif
