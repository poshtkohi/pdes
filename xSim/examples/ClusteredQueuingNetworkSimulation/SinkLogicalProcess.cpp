/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "SinkLogicalProcess.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            SinkLogicalProcess::SinkLogicalProcess(const String &objName, Int32 numOfInputs, Int32 numOfPatterns, Int32 criticalDelayPath): objName(objName), numOfInputs(numOfInputs), numOfPatterns(numOfPatterns), criticalDelayPath(criticalDelayPath)
            {
                disposed = false;
            }
            //----------------------------------------------------
            SinkLogicalProcess::~SinkLogicalProcess()
            {
                if(!disposed)
                {
                    disposed = true;
                }
            }
			//----------------------------------------------------
			void SinkLogicalProcess::Initialize()
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
			void SinkLogicalProcess::Finalize()
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
            void SinkLogicalProcess::ExecuteProcess()
            {
				QueueEvent *queueEvent = null;

				//while(HaveMoreEvents())
				{
					queueEvent = (QueueEvent *)GetNextEvent();
					//queueEvent = (QueueEvent *)PeekNextEvent();

					if(queueEvent != null)
					{
#ifdef __Parvicursor_xSim_Debug_Enable__
						*logFile << "At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
						//cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
#endif

						//cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///

                        if(numOfInputs == 1)
						{
                            UInt64 simTime = GetSimulationTime();
                            UInt64 sendTime = simTime;
                            UInt64 recvTime = simTime;// + 1;//criticalDelayPath * (numOfPatterns + 1);
                            for(register Int32 i = 0  ; i < GetOutputLpCount() ; i++)
                            {
                                QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this->GetID(), GetOutputLpId(i));
                                newEvent->frame = queueEvent->frame;
                                SendEvent(newEvent);
                            }

                            numEventProcessed++;
                            //delete queueEvent;
						}
						else
						{
                            frames[queueEvent->frame] = frames[queueEvent->frame] + 1;

                            //printf("frames[%d]: %d\n", queueEvent->frame, frames[queueEvent->frame]);
                            if(frames[queueEvent->frame] == numOfInputs)
                            {
                                UInt64 simTime = GetSimulationTime();
                                UInt64 sendTime = simTime;
                                UInt64 recvTime = simTime;// + 1;//criticalDelayPath * (numOfPatterns + 1);
								//Int64 recvTime = simTime + criticalDelayPath;
                                for(register Int32 i = 0  ; i < GetOutputLpCount() ; i++)
                                {
                                    QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this->GetID(), GetOutputLpId(i));
                                    newEvent->frame = queueEvent->frame;
                                    SendEvent(newEvent);
                                    //printf("sink i: %d Send To: %s recvTime: %d frame: %d\n", i, GetLogicalProcessById(GetOutputLpId(i))->GetName().get_BaseStream(), recvTime, queueEvent->frame);
                                }
                                ///*logFile << "At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
                                frames[queueEvent->frame] = 0;
                            }

                            numEventProcessed++;
                           // delete queueEvent;
						}
					}
				}
            }
            //----------------------------------------------------
            const String &SinkLogicalProcess::GetName()
            {
                return objName;
            }
			//----------------------------------------------------
			State *SinkLogicalProcess::GetState()
			{
				return myState;
			}
			//----------------------------------------------------
			State *SinkLogicalProcess::AllocateState()
			{
				return (State *)new SinkState();
			}
			//----------------------------------------------------
			void SinkLogicalProcess::DeallocateState(const State *state)
			{
				delete (SinkState *)state;
			}
			//----------------------------------------------------
			void SinkLogicalProcess::DeallocateEvent(const EventInterface *e)
			{
				delete (QueueEvent *)e;
			}
            //----------------------------------------------------
            void SinkLogicalProcess::PrintInformation()
            {
				cout << "------------------------------------------------\n";
				cout << "The logical process '" << GetName().get_BaseStream() << "' was successfully instantiated.\n";
				cout << "Information of Logical Process '" << GetName().get_BaseStream() << "':\n";
				cout << "GetID: " << GetID() << "\n";
				cout << "GetOutputLpCount(): " << GetOutputLpCount() << "\n";
				cout << "numOfInputs: " << numOfInputs << "\n";
				cout << endl;
            }
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
