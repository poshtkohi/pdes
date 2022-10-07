/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "MonitorLogicalProcess.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            MonitorLogicalProcess::MonitorLogicalProcess(const String &objName, Int32 numberOfInputs): objName(objName), numberOfInputs(numberOfInputs)
            {
                disposed = false;
            }
            //----------------------------------------------------
            MonitorLogicalProcess::~MonitorLogicalProcess()
            {
                if(!disposed)
                {
                    disposed = true;
                }
            }
			//----------------------------------------------------
			void MonitorLogicalProcess::Initialize()
			{
				inputBits = new Int32[numberOfInputs];
				for(Int32 i = 0 ; i < numberOfInputs ; i++)
					inputBits[i] = -1 - i;

				numEventProcessed = 0;
				numInputValue = 0;

				logFile = new ofstream(("out/" + GetName() + ".txt").get_BaseStream(), ios::out); ///

				if(!logFile->is_open())
					throw IOException("Could not open the file out/" + GetName() + ".txt");
			}
			//----------------------------------------------------
			void MonitorLogicalProcess::Finalize()
			{
				delete inputBits; inputBits = null;
				logFile->flush(); ///
				logFile->close(); ///
				delete logFile; ///
			}
            //----------------------------------------------------
            void MonitorLogicalProcess::ExecuteProcess()
            {
				LogicEvent *inputEvent = null;

				while(HaveMoreEvents())
				{
					inputEvent = (LogicEvent *)GetNextEvent();
		
					if(inputEvent != null)
					{
						if(numInputValue != numberOfInputs)
						{
							inputBits[inputEvent->destinationPort] = inputEvent->bitValue;
							frame = inputEvent->frame; // Checks here for correct frame for all input events.
							numInputValue++;
							numEventProcessed++;

							*logFile << "Local Time: " << GetSimulationTime() << " Receive Time: " << inputEvent->GetReceiveTime() << " Frame: " << inputEvent->frame << " Input bit: " << inputEvent->bitValue << " Port: " << inputEvent->destinationPort << endl; ///
							//cout << GetName().get_BaseStream() << " Local Time: " << GetSimulationTime() << " Receive Time: " << inputEvent->GetReceiveTime() << " Frame: " << inputEvent->frame << " Input bit: " << inputEvent->bitValue << " Port: " << inputEvent->destinationPort << endl; ///
							delete inputEvent;
						}

						if(numInputValue == numberOfInputs)
						{
							ShowResults();
							numInputValue = 0;

							/*for(register Int32 i = 0 ; i < numberOfInputs ; i++)
								inputBits[i] = -1;*/
						}
					}
				}
            }
            //----------------------------------------------------
            String MonitorLogicalProcess::GetName()
            {
                return objName;
            }
            //----------------------------------------------------
            void MonitorLogicalProcess::ShowResults()
            {
				*logFile << "Local Time: " << GetSimulationTime() << " Frame: " << frame << " Input bits: "; ///
				for(register Int32 i = 0 ; i < numberOfInputs ; i++) ///
					*logFile << inputBits[i]; ///

				*logFile << endl; ///

				/*cout << "ShowResults() for '" << GetName().get_BaseStream() << "' in Frame " << frame << " with localTime " << GetSimulationTime() << ": " ;
				for(register Int32 i = 0 ; i < numberOfInputs ; i++)
					cout << inputBits[i];*/

				cout << endl;
            }
            //----------------------------------------------------
            void MonitorLogicalProcess::PrintInformation()
            {
				cout << "------------------------------------------------\n";
				cout << "The logical process '" << GetName().get_BaseStream() << "' was successfully instantiated.\n";
				cout << "Information of Logical Process '" << GetName().get_BaseStream() << "':\n";
				cout << "GetID: " << GetID() << "\n";
				cout << "numberOfInputs: " << numberOfInputs;
				cout << endl;
            }
            //----------------------------------------------------
            Int32 MonitorLogicalProcess::GetObjectDelay()
            {
                return 0;
            }
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
