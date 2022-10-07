/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "SinkObject.h"
#include <iostream>

//----------------------------------------------------
SinkObject::SinkObject(string &objName, int numOfInputs, int criticalDelayPath, int numOfPatterns, int numberOfOutputs, int objectDelay, vector<string> *outputNames)
                        : objName(objName), numOfInputs(numOfInputs), criticalDelayPath(criticalDelayPath), numOfPatterns(numOfPatterns), numberOfOutputs(numberOfOutputs), objectDelay(objectDelay), outputNames(outputNames)
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
    //logFile.open("out/" + getName() + ".txt"); ///

    ///cout << endl;
    ///cout << "this is " << getName() << endl;
    SinkState *newState = (SinkState *) getState();

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

    log = getOFStream("out/" + getName() + ".txt", ios::out);

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
            //logFile << "At LVT: " << getSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
            //cout << getName() << " At LVT: " << getSimulationTime() << " Frame: " << queueEvent->frame << endl; ///

            //cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///

            if(numOfInputs == 1)
            {
                IntVTime simTime = static_cast<const IntVTime&>(getSimulationTime());
                IntVTime sendTime = simTime;
                //IntVTime recvTime = simTime + 1;//criticalDelayPath * (numOfPatterns + 1);
                IntVTime recvTime = simTime + criticalDelayPath;
                for(register int i = 0  ; i < numberOfOutputs ; i++)
                {
                    SimulationObject *receiver = outputHandles[i];
                    QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this, receiver);
                    newEvent->frame = queueEvent->frame;
                    //newEvent->row = queueEvent->row;
                    receiver->receiveEvent(newEvent);
                    //cout << "receiever1" << receiver->getName() << endl;
                }

                ///delete queueEvent;
            }
            else
            {
                state->frames[queueEvent->frame] = state->frames[queueEvent->frame] + 1;

                if(state->frames[queueEvent->frame] == numOfInputs)
                {
                    IntVTime simTime = static_cast<const IntVTime&>(getSimulationTime());
                    IntVTime sendTime = simTime;
                    //IntVTime recvTime = simTime + 1;//criticalDelayPath * (numOfPatterns + 1);
                    IntVTime recvTime = simTime + criticalDelayPath;
                    for(register int i = 0  ; i < numberOfOutputs ; i++)
                    {
                        SimulationObject *receiver = outputHandles[i];
                        QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this, receiver);
                        newEvent->frame = queueEvent->frame;
                        //newEvent->row = queueEvent->row;
                        receiver->receiveEvent(newEvent);
                        //cout << "receiever2" << receiver->getName() << endl;
                        //cout << "in sink row: " << newEvent->row << " frame: " << newEvent->frame << " receiever: " << newEvent->getReceiver() << endl;
                    }

                    //logFile << "At LVT: " << getSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
                    ostringstream outstream;
                    outstream << "At LVT: " << getSimulationTime() << " Frame: " << queueEvent->frame << "\n";

                    log->insert(outstream);
                    log->flush();
                    state->frames[queueEvent->frame] = 0;
                }

                ///delete queueEvent;
            }
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
    SEVERITY severity = NOTE;
    //simulation is over
    //let's see how we did
    SinkState* myState = static_cast<SinkState*>(getState());
    //ASSERT(myState != NULL);
    string msg = objName + " " + myState->getSummaryString() + "\n";
    reportError(msg, severity);


    //logFile.flush(); ///
    //logFile.close(); ///
}
//----------------------------------------------------
State *SinkObject::allocateState()
{
    return (State *) new SinkState(numOfInputs, numOfPatterns);
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
    cout << "Information of Simulation Object '" << getName() << "':\n";
    cout << "numOfOutputs: " << numberOfOutputs << "\n";
    cout << "numOfInputs: " << numOfInputs << "\n";
    cout << "outputNames: ";
    for(register int i = 0 ; i < numberOfOutputs ; i++)
        cout << (*outputNames)[i] << ", ";
    cout << "\n" << endl;
}
//----------------------------------------------------
