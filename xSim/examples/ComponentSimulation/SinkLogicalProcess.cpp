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
            SinkLogicalProcess::SinkLogicalProcess(const String &objName): objName(objName)
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
				//numEventProcessed = 0;
				myState = (SinkState *)AllocateState();

//#ifdef __Parvicursor_xSim_Debug_Enable__
				logFile = new ofstream(("out/" + GetName() + ".txt").get_BaseStream(), ios::out); ///

				if(!logFile->is_open())
					throw IOException("Could not open the file out/" + GetName() + ".txt");
//#endif
			}
			//----------------------------------------------------
			void SinkLogicalProcess::Finalize()
			{
//#ifdef __Parvicursor_xSim_Debug_Enable__
				logFile->flush(); ///
				logFile->close(); ///
				delete logFile; ///
//#endif
				DeallocateState(myState);
			}
            //----------------------------------------------------
            void SinkLogicalProcess::ExecuteProcess()
            {
				QueueEvent *queueEvent = (QueueEvent *)GetNextEvent();

				if(queueEvent != null)
				{
//#ifdef __Parvicursor_xSim_Debug_Enable__
					*logFile << "At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
					//cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " Frame: " << queueEvent->frame << endl; ///
//#endif

					myState->numEventProcessed++;
					// We must not delete the queueEvenet here because the xSim simulation manager automatically
					// releases the events.
					///delete queueEvent;
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
				cout << endl;
            }
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
