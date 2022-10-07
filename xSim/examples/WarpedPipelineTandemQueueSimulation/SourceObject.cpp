/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "SourceObject.h"
#include <iostream>

//----------------------------------------------------
SourceObject::SourceObject(string &objName, int numOfPatterns, int criticalDelayPath, int numberOfOutputs, vector<string> *outputNames)
                        : objName(objName), numOfPatterns(numOfPatterns), criticalDelayPath(criticalDelayPath), numberOfOutputs(numberOfOutputs), outputNames(outputNames)
{
    disposed = true;
}
//----------------------------------------------------
SourceObject::~SourceObject()
{
}
//----------------------------------------------------
void SourceObject::initialize()
{
    ///logFile.open("out/" + getName() + ".txt"); ///

    ///cout << endl;
    ///cout << "this is " << getName() << endl;
    SourceState *newState = (SourceState *) getState();

    if(numberOfOutputs != 0)
    {
        outputHandles = new SimulationObject *[numberOfOutputs];
        for (int i = 0; i < numberOfOutputs; i++)
        {
          outputHandles[i] = getObjectHandle((*outputNames)[i]);
          ///cout << "the server's des object is " << (*outputNames)[i] << endl;
        }
    }

    frame = 0;

    IntVTime simTime = static_cast<const IntVTime&>(getSimulationTime());
    IntVTime sendTime = simTime;
    IntVTime recvTime = simTime;

    QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this, this);
    newEvent->frame = frame;
    this->receiveEvent(newEvent);

    frame++;

    disposed = false;
}
//----------------------------------------------------
void SourceObject::executeProcess()
{
    ///cout << endl;
    ///cout << "in the executePorcess() " << getName() << endl;

    SourceState *state = static_cast<SourceState*>(getState());

    QueueEvent *queueEvent = NULL;

    while(haveMoreEvents() == true)
    {
        //we got an event
        //let's get the event
        queueEvent = (QueueEvent *)getEvent();
        if(queueEvent != NULL)
        {
            ///logFile << "At LVT: " << getSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
            ///cout << getName() << " At LVT: " << getSimulationTime() << " Frame: " << queueEvent->frame << endl; ///

            IntVTime simTime = static_cast<const IntVTime&>(getSimulationTime());
            IntVTime sendTime = simTime;
            IntVTime recvTime = simTime + 1;

            for(register int i = 0  ; i < numberOfOutputs ; i++)
            {
                SimulationObject *receiver = outputHandles[i];
                QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this, receiver);
                newEvent->frame = queueEvent->frame;
                receiver->receiveEvent(newEvent);
            }

            ///delete queueEvent;

            if(frame < numOfPatterns)
            {
                IntVTime simTime = static_cast<const IntVTime&>(getSimulationTime());
                IntVTime sendTime = simTime;
                IntVTime recvTime = simTime + 1;// + criticalDelayPath + 1;


                QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this, this);
                newEvent->frame = frame;
                this->receiveEvent(newEvent);

                frame++;
            }
        }
        state->EventProcessed();
    }
}
//----------------------------------------------------
void SourceObject::finalize()
{
    if(disposed)
        return;

    disposed = true;
    /*SEVERITY severity = NOTE;
    //simulation is over
    //let's see how we did
    SourceState* myState = static_cast<SourceState*>(getState());
    //ASSERT(myState != NULL);
    string msg = objName + " " + myState->getSummaryString() + "\n";
    reportError(msg, severity);*/


    ///logFile.flush(); ///
    ///logFile.close(); ///
}
//----------------------------------------------------
State *SourceObject::allocateState()
{
    return (State *) new SourceState();
}
//----------------------------------------------------
void SourceObject::deallocateState(const State *state)
{
    delete (SourceState *)state;
}
//----------------------------------------------------
void SourceObject::reclaimEvent(const Event *event)
{
    // delete event
    // HINT: you could insert this in a free pool of event
    delete (QueueEvent *)event;
}
//----------------------------------------------------
void SourceObject::PrintInformation()
{
    cout << "------------------------------------------------\n";
    cout << "The simulation object '" << getName() << "' was successfully instantiated.\n";
    cout << "Information of Simulation Object '" << getName() << "':\n";
    cout << "numOfOutputs: " << numberOfOutputs << "\n";
    cout << "outputNames: ";
    for(register int i = 0 ; i < numberOfOutputs ; i++)
        cout << (*outputNames)[i] << ", ";
    cout << "\n" << endl;
}
//----------------------------------------------------
