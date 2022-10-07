/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "SourceLogicalProcess.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            SourceLogicalProcess::SourceLogicalProcess(const String &objName, Int32 numOfInputs, Int32 numOfPatterns, Int32 criticalDelayPath) : objName(objName), numOfInputs(numOfInputs), numOfPatterns(numOfPatterns), criticalDelayPath(criticalDelayPath)
            {
            }
            //----------------------------------------------------
            SourceLogicalProcess::~SourceLogicalProcess()
            {
                if(!disposed)
                {
                    disposed = true;
                }
            }
			//----------------------------------------------------
			void SourceLogicalProcess::Initialize()
			{
#ifdef __Parvicursor_xSim_Debug_Enable__
				logFile = new ofstream(("out/" + GetName() + ".txt").get_BaseStream(), ios::out); ///

				if(!logFile->is_open())
					throw IOException("Could not open the file out/" + GetName() + ".txt");
#endif

				frame = 0;
				numEventProcessed = 0;
				numValue = 0;

                UInt64 simTime = GetSimulationTime();
                UInt64 sendTime = simTime;
                UInt64 recvTime = simTime;

				for(register Int32 i  = 0 ; i < numOfPatterns ; i++)
				{
                    sendTime = recvTime;
                    recvTime = sendTime + 1; //criticalDelayPath;
					//recvTime = sendTime + criticalDelayPath;

                    QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this->GetID(), this->GetID());
                    newEvent->frame = i;
                    SendEvent(newEvent);
                    //printf("Generating input in frame %d recvTime %d\n", i , recvTime);
				}
			}
			//----------------------------------------------------
			void SourceLogicalProcess::Finalize()
			{
#ifdef __Parvicursor_xSim_Debug_Enable__
				logFile->flush(); ///
				logFile->close(); ///
				delete logFile; ///
#endif
			}
			//----------------------------------------------------
			void SourceLogicalProcess::ExecuteProcess()
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

						UInt64 simTime = GetSimulationTime();
						UInt64 sendTime = simTime;
						UInt64 recvTime = simTime + 1; //simTime;

						for(register Int32 i = 0  ; i < GetOutputLpCount() ; i++)
						{
							QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this->GetID(), GetOutputLpId(i));
							newEvent->frame = queueEvent->frame;
							SendEvent(newEvent);
							//printf("source i: %d Send To: %s recvTime: %d\n", i, GetLogicalProcessById(GetOutputLpId(i))->GetName().get_BaseStream(), recvTime);
						}

						numEventProcessed++;
						//delete queueEvent;
					}
				}
			}
			//----------------------------------------------------
			const String &SourceLogicalProcess::GetName()
			{
				return objName;
			}
			//----------------------------------------------------
			State *SourceLogicalProcess::GetState()
			{
				return myState;
			}
			//----------------------------------------------------
			State *SourceLogicalProcess::AllocateState()
			{
				return (State *)new SourceState();
			}
			//----------------------------------------------------
			void SourceLogicalProcess::DeallocateState(const State *state)
			{
				delete (SourceState *)state;
			}
			//----------------------------------------------------
			void SourceLogicalProcess::DeallocateEvent(const EventInterface *e)
			{
				delete (QueueEvent *)e;
			}
            //----------------------------------------------------
            void SourceLogicalProcess::PrintInformation()
            {
				cout << "------------------------------------------------\n";
				cout << "The logical process '" << GetName().get_BaseStream() << "' was successfully instantiated.\n";
				cout << "Information of Logical Process '" << GetName().get_BaseStream() << "':\n";
				cout << "GetID: " << GetID() << "\n";
				cout << "GetOutputLpCount(): " << GetOutputLpCount() << "\n";
				cout << "numOfInputs: " << numOfInputs << "\n";
				//cout << "------------------------------------------------";
				cout << endl;
            }
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
