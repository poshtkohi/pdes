/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "ServerObject.h"
#include <iostream>

//----------------------------------------------------
ServerObject::ServerObject(string &objName, int numOfInputs, int criticalDelayPath, int numOfPatterns, int numberOfOutputs, int objectDelay, vector<string> *outputNames)
                        : objName(objName), numOfInputs(numOfInputs), criticalDelayPath(criticalDelayPath), numOfPatterns(numOfPatterns), numberOfOutputs(numberOfOutputs), objectDelay(objectDelay), outputNames(outputNames)
{
    disposed = true;
}
//----------------------------------------------------
ServerObject::~ServerObject()
{
}
//----------------------------------------------------
void ServerObject::initialize()
{
    ///logFile.open("out/" + getName() + ".txt"); ///

    ///cout << endl;
    ///cout << "this is " << getName() << endl;
    ServerState *newState = (ServerState *) getState();

    ///cout << "numberOfInputs is:" << numOfInputs << endl;

    if(numberOfOutputs != 0)
    {
        outputHandles = new SimulationObject *[numberOfOutputs];
        for (int i = 0; i < numberOfOutputs; i++)
        {
          outputHandles[i] = getObjectHandle((*outputNames)[i]);
          ///cout << "the server's des object is " << (*outputNames)[i] << endl;
        }
    }

    disposed = false;
}
//----------------------------------------------------
void ServerObject::executeProcess()
{
    ///cout << endl;
    ///cout << "in the executePorcess() " << getName() << endl;

    ServerState *state = static_cast<ServerState*>(getState());

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

            //cout << "row: " << queueEvent->row << endl;

            if(numOfInputs == 1)
            {
                IntVTime simTime = static_cast<const IntVTime&>(getSimulationTime());
                IntVTime sendTime = simTime;
                IntVTime recvTime = simTime + objectDelay;
                for(register int i = 0  ; i < numberOfOutputs ; i++)
                {
                    SimulationObject *receiver = outputHandles[i];
                    QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this, receiver);
                    newEvent->frame = queueEvent->frame;
                    //newEvent->row = queueEvent->row;
                    receiver->receiveEvent(newEvent);
                }

                ///delete queueEvent;
            }
            else
            {
                state->frames[queueEvent->frame] = state->frames[queueEvent->frame] + 1;

                if(state->frames[queueEvent->frame] == 1)
                {
                    IntVTime simTime = static_cast<const IntVTime&>(getSimulationTime());
                    IntVTime sendTime = simTime;
                    IntVTime recvTime = simTime + objectDelay;
                    for(register int i = 0  ; i < numberOfOutputs ; i++)
                    {
                        SimulationObject *receiver = outputHandles[i];
                        QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this, receiver);
                        newEvent->frame = queueEvent->frame;
                        //newEvent->row = queueEvent->row;
                        receiver->receiveEvent(newEvent);
                    }
                }
                else if(state->frames[queueEvent->frame] == numOfInputs)
                {
                    state->frames[queueEvent->frame] = 0;
                }

                ///delete queueEvent;
            }
        }
        state->EventProcessed();
    }
}
//----------------------------------------------------
void ServerObject::finalize()
{
    if(disposed)
        return;

    disposed = true;
    SEVERITY severity = NOTE;
    //simulation is over
    //let's see how we did
    ServerState* myState = static_cast<ServerState*>(getState());
    //ASSERT(myState != NULL);
    string msg = objName + " " + myState->getSummaryString() + "\n";
    reportError(msg, severity);


    ///logFile.flush(); ///
    ///logFile.close(); ///
}
//----------------------------------------------------
State *ServerObject::allocateState()
{
    return (State *) new ServerState(numOfInputs, numOfPatterns);
}
//----------------------------------------------------
void ServerObject::deallocateState(const State *state)
{
    delete (ServerState *)state;
}
//----------------------------------------------------
void ServerObject::reclaimEvent(const Event *event)
{
    // delete event
    // HINT: you could insert this in a free pool of event
    delete (QueueEvent *)event;
}
//----------------------------------------------------
void ServerObject::PrintInformation()
{
    cout << "------------------------------------------------\n";
    cout << "The simulation object '" << getName() << "' was successfully instantiated.\n";
    cout << "Information of Simulation Object '" << getName() << "':\n";
    cout << "numOfOutputs: " << numberOfOutputs << "\n";
    cout << "numOfInputs: " << numOfInputs << "\n";
    cout << "outputNames: ";
    for(register int i = 0 ; i < numberOfOutputs ; i++)
        cout << (*outputNames)[i] << ", ";
    cout << "\n" << endl;
}
//----------------------------------------------------
