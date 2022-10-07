/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "SinkObject.h"
#include <iostream>

//----------------------------------------------------
SinkObject::SinkObject(string &objName) : objName(objName)
{
    disposed = true;
}
//----------------------------------------------------
SinkObject::~SinkObject()
{
}
//----------------------------------------------------
void SinkObject::initialize()
{
    logFile.open("out/" + getName() + ".txt"); ///

    ///cout << endl;
    ///cout << "this is " << getName() << endl;
    SinkState *newState = (SinkState *) getState();

    ///cout << "numberOfInputs is:" << numOfInputs << endl;

    //log = getOFStream("out/" + getName() + ".txt", ios::out);

    disposed = false;
}
//----------------------------------------------------
void SinkObject::executeProcess()
{
    ///cout << endl;
    ///cout << "in the executePorcess() " << getName() << endl;

    SinkState *state = static_cast<SinkState*>(getState());

    QueueEvent *queueEvent = NULL;

    while(haveMoreEvents() == true)
    {
        //we got an event
        //let's get the event
        queueEvent = (QueueEvent *)getEvent();
        if(queueEvent != NULL)
        {
            //cout << "row: " << queueEvent->row << endl;
            logFile << "At LVT: " << getSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
            //cout << getName() << " At LVT: " << getSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
            /*ostringstream outstream;
            outstream << "At LVT: " << getSimulationTime() << " Frame: " << queueEvent->frame << "\n";
            log->insert(outstream);
            log->flush();*/
            ///delete queueEvent;
        }
        state->EventProcessed();
    }
}
//----------------------------------------------------
void SinkObject::finalize()
{
    if(disposed)
        return;

    disposed = true;
    /*SEVERITY severity = NOTE;
    //simulation is over
    //let's see how we did
    SinkState* myState = static_cast<SinkState*>(getState());
    //ASSERT(myState != NULL);
    string msg = objName + " " + myState->getSummaryString() + "\n";
    reportError(msg, severity);*/


    //logFile.flush(); ///
    //logFile.close(); ///
}
//----------------------------------------------------
State *SinkObject::allocateState()
{
    return (State *) new SinkState();
}
//----------------------------------------------------
void SinkObject::deallocateState(const State *state)
{
    delete (SinkState *)state;
}
//----------------------------------------------------
void SinkObject::reclaimEvent(const Event *event)
{
    // delete event
    // HINT: you could insert this in a free pool of event
    delete (QueueEvent *)event;
}
//----------------------------------------------------
void SinkObject::PrintInformation()
{
    cout << "------------------------------------------------\n";
    cout << "The simulation object '" << getName() << "' was successfully instantiated.\n";
    cout << "Information of Simulation Object '" << getName() << "':\n" << endl;
}
//----------------------------------------------------
