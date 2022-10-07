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
            SourceLogicalProcess::SourceLogicalProcess(const String &objName) : objName(objName), counter(0)
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
/*#ifdef __Parvicursor_xSim_Debug_Enable__
				logFile = new ofstream(("out/" + GetName() + ".txt").get_BaseStream(), ios::out); ///

				if(!logFile->is_open())
					throw IOException("Could not open the file out/" + GetName() + ".txt");
#endif*/

				//cout << "SourceLogicalProcess::Initialize()" << endl;
				counter = 1;
				Int64 simTime = GetSimulationTime();
				Int64 sendTime = simTime;
				Int64 recvTime = simTime;// + 3;

				QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this, this);
				newEvent->counter = counter;
				SendEvent(newEvent);

				//counter++;
			}
			//----------------------------------------------------
			void SourceLogicalProcess::Finalize()
			{
/*#ifdef __Parvicursor_xSim_Debug_Enable__
				logFile->flush(); ///
				logFile->close(); ///
				delete logFile; ///
#endif*/
			}
			//----------------------------------------------------
			void SourceLogicalProcess::ExecuteProcess()
			{
				QueueEvent *queueEvent = (QueueEvent *)GetNextEvent();

				if(queueEvent != null)
				{
/*#ifdef __Parvicursor_xSim_Debug_Enable__
					*logFile << "At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
					//cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
#endif*/
                    //cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " Counter: " << queueEvent->counter << endl; ///

					Int64 simTime = GetSimulationTime();
					Int64 sendTime = simTime;
					Int64 recvTime = simTime + 1;

					for(register Int32 i = 0  ; i < GetOutputLpCount() ; i++)
					{
						QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this, GetLogicalProcessById(GetOutputLpId(i)));
						newEvent->counter = counter;
						SendEvent(newEvent);
					}

					//cout << "@time " << GetSimulationTime() << " x " << counter << " at source" << endl; ///
					simTime = GetSimulationTime();
					sendTime = simTime;
					recvTime = simTime + 3;

					QueueEvent *newEvent = new QueueEvent(sendTime, recvTime, this, this);
					//newEvent->counter = counter;
					SendEvent(newEvent);

					counter++;
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
				return null;
				//return myState;
			}
			//----------------------------------------------------
			State *SourceLogicalProcess::AllocateState()
			{
				return null;
				//return (State *)new SourceState();
			}
			//----------------------------------------------------
			void SourceLogicalProcess::DeallocateState(const State *state)
			{
				//delete (SourceState *)state;
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
				cout << "------------------------------------------------";
				cout << endl;
            }
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
