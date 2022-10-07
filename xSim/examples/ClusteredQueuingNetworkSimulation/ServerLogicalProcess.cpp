/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "ServerLogicalProcess.h"


//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            ServerLogicalProcess::ServerLogicalProcess(const String &objName, Int32 numOfInputs, Int32 objectDelay, Int32 criticalDelayPath, Int32 numOfPatterns) : objName(objName), numOfInputs(numOfInputs), objectDelay(objectDelay), criticalDelayPath(criticalDelayPath), numOfPatterns(numOfPatterns)
            {
                disposed = false;
            }
            //----------------------------------------------------
            ServerLogicalProcess::~ServerLogicalProcess()
            {
                if(!disposed)
                {
                    disposed = true;
                }
            }
			//----------------------------------------------------
			void ServerLogicalProcess::Initialize()
			{
				numEventProcessed = 0;
				numValue = 0;
				frames = null;
				if(numOfInputs > 1)
				{
                    frames = new Int32[numOfPatterns];
                    for(register Int32 i = 0 ; i < numOfPatterns ; i++)
                        frames[i] = 0;
                }

#ifdef __Parvicursor_xSim_Debug_Enable__
				logFile = new ofstream(("out/" + GetName() + ".txt").get_BaseStream(), ios::out); ///

				if(!logFile->is_open())
					throw IOException("Could not open the file out/" + GetName() + ".txt");
#endif
			}
			//----------------------------------------------------
			void ServerLogicalProcess::Finalize()
			{
                if(frames != null)
                {
                    delete frames;
                    frames = null;
                }
#ifdef __Parvicursor_xSim_Debug_Enable__
				logFile->flush(); ///
				logFile->close(); ///
				delete logFile; ///
#endif
			}
            //----------------------------------------------------
            void ServerLogicalProcess::ExecuteProcess()
            {
				QueueEvent *queueEvent = null;

				//while(HaveMoreEvents())
				{
					queueEvent = (QueueEvent *)GetNextEvent();

					if(queueEvent != null)
					{
#ifdef __Parvicursor_xSim_Debug_Enable__
						*logFile << "At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
						cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
#endif

						//cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///

						if(numOfInputs == 1)
						{
                            UInt64 simTime = GetSimulationTime();
                            UInt64 sendTime = simTime;
                            UInt64 recvTime = simTime + objectDelay;
                            for(register Int32 i = 0  ; i < GetOutputLpCount() ; i++)
                            {
                                QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this->GetID(), GetOutputLpId(i));
                                newEvent->frame = queueEvent->frame;
                                SendEvent(newEvent);
                                ///printf("i: %d Send To: %s recvTime: %d\n", i, GetLogicalProcessById(GetOutputLpId(i))->GetName().get_BaseStream(), recvTime);
                            }

                            numEventProcessed++;
                            //delete queueEvent;
						}
						else
						{
                            frames[queueEvent->frame] = frames[queueEvent->frame] + 1;

                            if(frames[queueEvent->frame] == 1)
                            {
                                UInt64 simTime = GetSimulationTime();
                                UInt64 sendTime = simTime;
                                UInt64 recvTime = simTime + objectDelay;
                                for(register Int32 i = 0  ; i < GetOutputLpCount() ; i++)
                                {
                                    QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this->GetID(), GetOutputLpId(i));
                                    newEvent->frame = queueEvent->frame;
                                    SendEvent(newEvent);
                                    ///printf("i: %d Send To: %s recvTime: %d\n", i, GetLogicalProcessById(GetOutputLpId(i))->GetName().get_BaseStream(), recvTime);
                                }
                            }
                            else if(frames[queueEvent->frame] == numOfInputs)
                            {
                                frames[queueEvent->frame] = 0;
                            }

                            numEventProcessed++;
                            //delete queueEvent;
						}
					}
				}
            }
            //----------------------------------------------------
            const String &ServerLogicalProcess::GetName()
            {
                return objName;
            }
			//----------------------------------------------------
			State *ServerLogicalProcess::GetState()
			{
				return myState;
			}
			//----------------------------------------------------
			State *ServerLogicalProcess::AllocateState()
			{
				return (State *)new ServerState();
			}
			//----------------------------------------------------
			void ServerLogicalProcess::DeallocateState(const State *state)
			{
				delete (ServerState *)state;
			}
			//----------------------------------------------------
			void ServerLogicalProcess::DeallocateEvent(const EventInterface *e)
			{
				delete (QueueEvent *)e;
			}
			//----------------------------------------------------
            void ServerLogicalProcess::PrintInformation()
            {
				cout << "------------------------------------------------\n";
				cout << "The logical process '" << GetName().get_BaseStream() << "' was successfully instantiated.\n";
				cout << "Information of Logical Process '" << GetName().get_BaseStream() << "':\n";
				cout << "GetID: " << GetID() << "\n";
				cout << "GetOutputLpCount(): " << GetOutputLpCount() << "\n";
				cout << "ObjectDelay: " << objectDelay << "\n";
				cout << "numOfInputs: " << numOfInputs << "\n";
				cout << endl;
            }
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
