/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "CircleLogicalProcess.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            CircleLogicalProcess::CircleLogicalProcess(const String &objName) : objName(objName)
            {
                disposed = false;
            }
            //----------------------------------------------------
            CircleLogicalProcess::~CircleLogicalProcess()
            {
                if(!disposed)
                {
                    disposed = true;
                }
            }
			//----------------------------------------------------
			void CircleLogicalProcess::Initialize()
			{
				logFile = new ofstream(("out/" + GetName() + ".txt").get_BaseStream(), ios::out); ///

				if(GetID() == 0)
				{
					Int32 data = Random(100).Next();
					Int64 simTime = GetSimulationTime();
					Int64 sendTime = simTime;
					DefaultEvent *selfEvent = new DefaultEvent(sendTime , sendTime + 1, this, this, (Object *)data);
					SendEvent(selfEvent);
					//cout << "At " <<  GetName().get_BaseStream() << " @Timestamp: " << GetSimulationTime() << " Data: " << data << endl;
					//*logFile << "@Timestamp: " << GetSimulationTime() << GetID() << " Data: " << data << endl;
				}
			}
			//----------------------------------------------------
			void CircleLogicalProcess::Finalize()
			{
				logFile->flush(); ///
				logFile->close(); ///
				delete logFile; ///
			}
            //----------------------------------------------------
            void CircleLogicalProcess::ExecuteProcess()
            {
				while(HaveMoreEvents())
				{
					if(GetID() == 0)
					{
						// Receive events from myself and send them.
						DefaultEvent *selfEvent = (DefaultEvent *)GetNextEvent();
						if(selfEvent == null)
							continue;
						Int64 simTime = GetSimulationTime();
						Int64 sendTime = simTime;
						Int64 recvTime = sendTime + 2;
						DefaultEvent *newEvent = new DefaultEvent(sendTime, recvTime, this, GetLogicalProcessById(1), selfEvent->GetData());
						SendEvent(newEvent);
						cout << "At " <<  GetName().get_BaseStream() << " @Timestamp: " << GetSimulationTime() << " Data: " << (Int32)selfEvent->GetData() << endl;
						*logFile << "@Timestamp: " << GetSimulationTime() << " Data: " << (Int32)selfEvent->GetData() << endl;
						delete selfEvent;

						// Sends an event to myself
						//sendTime = simTime;
						//recvTime = sendTime;
						Int32 data = Random(recvTime).Next();
						selfEvent = new DefaultEvent(sendTime, sendTime + 1, this, this, (Object *)data);
						SendEvent(selfEvent);
					}
					if(GetID() == 1)
					{
						DefaultEvent *nextEvent = (DefaultEvent *)GetNextEvent();
						if(nextEvent == null)
							continue;
						cout << "At " <<  GetName().get_BaseStream() << " @Timestamp: " << GetSimulationTime() << " Data: " << (Int32)nextEvent->GetData() << endl;
						*logFile << "@Timestamp: " << GetSimulationTime() << " Data: " << (Int32)nextEvent->GetData() << endl;
						delete nextEvent;
					}
				}
            }
            //----------------------------------------------------
            String CircleLogicalProcess::GetName()
            {
                return objName;
            }
			//----------------------------------------------------
			void CircleLogicalProcess::PrintInformation()
			{
				//cout << "------------------------------------------------\n";
				cout << "The logical process '" << GetName().get_BaseStream() << "' was successfully instantiated.\n";
				cout << "GetID: " << GetID() << "\n";

				if(GetInputLpCount() > 0)
				{
					cout << "Input Logical Processes: ";
					for(register Int32 i = 0 ; i < GetInputLpCount() ; i++)
						cout << GetInputLpId(i) << "\t";
					cout << endl;
				}

				if(GetOutputLpCount() > 0)
				{
					cout << "Output Logical Processes: ";
					for(register Int32 i = 0 ; i < GetOutputLpCount() ; i++)
						cout << GetOutputLpId(i) << "\t";
					cout << endl;
				}

				cout << endl;
			}
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
