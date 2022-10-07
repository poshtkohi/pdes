/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "ServerLogicalProcess.h"
#include <math.h>


//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            ServerLogicalProcess::ServerLogicalProcess(const String &objName, Process p) : objName(objName), p(p)
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
				in1 = in2 = 0;

				if(p == P3)
				{
					cout << "@time: " << GetSimulationTime() << " in1: " << in1 << " in2: " << in2 << endl;
				}
/*#ifdef __Parvicursor_xSim_Debug_Enable__
				logFile = new ofstream(("out/" + GetName() + ".txt").get_BaseStream(), ios::out); ///

				if(!logFile->is_open())
					throw IOException("Could not open the file out/" + GetName() + ".txt");
#endif*/
			}
			//----------------------------------------------------
			void ServerLogicalProcess::Finalize()
			{
/*#ifdef __Parvicursor_xSim_Debug_Enable__
				logFile->flush(); ///
				logFile->close(); ///
				delete logFile; ///
#endif*/
			}
            //----------------------------------------------------
            void ServerLogicalProcess::ExecuteProcess()
            {
				QueueEvent *queueEvent = (QueueEvent *)GetNextEvent();

				if(queueEvent != null)
				{
#ifdef __Parvicursor_xSim_Debug_Enable__
					//*logFile << "At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
					//cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
#endif
                    //cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///

					if(p == P1P2 || p == P1P2)
					{
						Int64 simTime = GetSimulationTime();
						Int64 sendTime = simTime;
						Int64 recvTime = simTime + 1;

						for(register Int32 i = 0  ; i < GetOutputLpCount() ; i++)
						{
							QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this, GetLogicalProcessById(GetOutputLpId(i)));
							newEvent->counter = queueEvent->counter;
							SendEvent(newEvent);
						}

						//cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " In: " << queueEvent->counter << endl; ///
						/*if(GetName() == "p1")
							cout << "@time " << GetSimulationTime() << " x " << queueEvent->counter << " at " << GetName().get_BaseStream() << endl;
						else if(GetName() == "p2")
							cout << "@time " << GetSimulationTime() << " y " << queueEvent->counter << " at " << GetName().get_BaseStream() << endl;*/
					}
					else if(p == P3)
					{
						if(queueEvent->GetSenderReference()->GetName() == "source")
							in1 = queueEvent->counter;
						else if(queueEvent->GetSenderReference()->GetName() == "p2")
							in2 = queueEvent->counter;
						//cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " In1: " << in1 << " In2: " << in2 << endl; ///
						cout << "@time: " << GetSimulationTime() << " in1: " << in1 << " in2: " << in2 << endl; ///
						//cout << "@time " << GetSimulationTime() << " x " << in1 << " z " << in2 << " at " << GetName().get_BaseStream() << endl;
					}
					else if(p == P4)
					{
						//cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " In: " << queueEvent->counter << endl; ///
						//cout << "@time " << GetSimulationTime() << " x " << queueEvent->counter << " at " << GetName().get_BaseStream() << endl;
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
				return null;
				//return myState;
			}
			//----------------------------------------------------
			State *ServerLogicalProcess::AllocateState()
			{
				return null;
				//return (State *)new ServerState();
			}
			//----------------------------------------------------
			void ServerLogicalProcess::DeallocateState(const State *state)
			{
				//delete (ServerState *)state;
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
				cout << endl;
            }
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
