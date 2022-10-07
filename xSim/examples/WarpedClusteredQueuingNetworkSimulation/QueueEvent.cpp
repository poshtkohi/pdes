/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "QueueEvent.h"

//----------------------------------------------------
QueueEvent::QueueEvent( const VTime &initSendTime,
                        const VTime &initRecvTime,
                        SimulationObject *initSender,
                        SimulationObject *initReceiver) :
                        DefaultEvent( initSendTime, initRecvTime, initSender, initReceiver ),
                        frame(0)//, row(-1)
{}
//----------------------------------------------------
QueueEvent::QueueEvent( const VTime &initSendTime,
                        const VTime &initRecvTime,
                        const ObjectID &initSender,
                        const ObjectID &initReceiver,
                        const unsigned int eventIdVal):
                        DefaultEvent(initSendTime, initRecvTime, initSender, initReceiver, eventIdVal),
                        frame(0)//, row(-1)
{}
//----------------------------------------------------
void QueueEvent::serialize( SerializedInstance *addTo ) const
{
    Event::serialize(addTo);
    addTo->addInt(frame);
    //addTo->addInt(row);
}
//----------------------------------------------------
Serializable *QueueEvent::deserialize( SerializedInstance *instance )
{
    VTime *sendTime = dynamic_cast<VTime *>(instance->getSerializable());
    VTime *receiveTime = dynamic_cast<VTime *>(instance->getSerializable());
    unsigned int senderSimManID = instance->getUnsigned();
    unsigned int senderSimObjID = instance->getUnsigned();
    unsigned int receiverSimManID = instance->getUnsigned();
    unsigned int receiverSimObjID = instance->getUnsigned();
    unsigned int eventId = instance->getUnsigned();

    ObjectID sender(senderSimObjID, senderSimManID);
    ObjectID receiver(receiverSimObjID, receiverSimManID);

    QueueEvent *event = new QueueEvent(*sendTime, *receiveTime, sender, receiver, eventId);

    event->frame = instance->getInt();
    //event->row = instance->getInt();

    delete sendTime;
    delete receiveTime;

    return event;
}
//----------------------------------------------------
bool QueueEvent::eventCompare( const Event* event ){
 QueueEvent *QueueE = (QueueEvent*) event;
 return (compareEvents(this, event) && frame == QueueE->frame /*&& row == QueueE->row*/);
 //return compareEvents(this, event);
}
//----------------------------------------------------
