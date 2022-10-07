/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "MonitorLogicalProcess.h"

//**************************************************************************************************************//

extern Vector<Vector<Int32> *> __clusters__;
extern Int32 *__null_messages__;

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            MonitorLogicalProcess::MonitorLogicalProcess(const System::String &objName, Int32 numberOfInputs): objName(objName), numberOfInputs(numberOfInputs)
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
					inputBits[i] = 0;

				logFile = new ofstream(("out/" + GetName() + ".txt").get_BaseStream(), ios::out); ///

				if(!logFile->is_open())
					throw IOException("Could not open the file out/" + GetName() + ".txt");

				//ShowResults();
			}
			//----------------------------------------------------
			void MonitorLogicalProcess::Finalize()
			{
				delete inputBits;
				inputBits = null;
				/*logFile->flush(); ///
				logFile->close(); ///
				delete logFile; ///*/
			}
            //----------------------------------------------------
            void MonitorLogicalProcess::ExecuteProcess()
            {
				LogicEvent *inputEvent = (LogicEvent *)GetNextEvent();

				if(inputEvent != null)
				{
					inputBits[inputEvent->destinationPort] = inputEvent->bitValue;
					frame = inputEvent->frame; // Checks here for correct frame for all input events.

					//*logFile << "Local Time: " << GetSimulationTime() << " Receive Time: " << inputEvent->GetReceiveTime() << " Frame: " << inputEvent->frame << " Input bit: " << inputEvent->bitValue << " Port: " << inputEvent->destinationPort << endl; ///
					//cout << GetName().get_BaseStream() << " Local Time: " << GetSimulationTime() << " Receive Time: " << inputEvent->GetReceiveTime() << " Frame: " << inputEvent->frame << " Input bit: " << inputEvent->bitValue << " Port: " << inputEvent->destinationPort << endl; ///
					ShowResults();
					//delete inputEvent;
				}

                /*bool normalMessageReceived = false;
                bool appNullMessageReceived = false;

                while(HaveMoreEvents())
                {
                    LogicEvent *inputEvent = (LogicEvent *)GetNextEvent();

                    if(inputEvent != null)
                    {
                        if(inputEvent->GetEventType() == NormalMessage)
                        {
                            inputBits[inputEvent->destinationPort] = inputEvent->bitValue;
                            frame = inputEvent->frame; // Checks here for correct frame for all input events.
                            normalMessageReceived = true;
                            //*logFile << "Local Time: " << GetSimulationTime() << " Receive Time: " << inputEvent->GetReceiveTime() << " Frame: " << inputEvent->frame << " Input bit: " << inputEvent->bitValue << " Port: " << inputEvent->destinationPort << endl; ///
                            //cout << GetName().get_BaseStream() << " Local Time: " << GetSimulationTime() << " Receive Time: " << inputEvent->GetReceiveTime() << " Frame: " << inputEvent->frame << " Input bit: " << inputEvent->bitValue << " Port: " << inputEvent->destinationPort << endl; ///
                        }
                        else if(inputEvent->GetEventType() == AppNullMessage)
                            appNullMessageReceived = true;
                    }
                }
                //Terminate();

                if(normalMessageReceived)
                {
                    //ShowResults();

                    if(GetSimulationManagerType() == DeadlockDetection)
                        SendNullMessages(); // Sends null messages
                }
                else
                    if(GetSimulationManagerType() == DeadlockDetection)
                        SendNullMessages(); // Sends null messages*/
            }
            //----------------------------------------------------
			void MonitorLogicalProcess::SendNullMessages()
			{
                return;
                __null_messages__[GetOwnerClusterID()]++;
                if(__null_messages__[GetOwnerClusterID()] != 200) //200
                    return;
                __null_messages__[GetOwnerClusterID()] = 0;

                Vector<Int32> *destClusters = __clusters__[GetOwnerClusterID()];
                for(register Int32 i = 0 ; i < destClusters->Size() ; i++)
                {
                    //std::cout << "Sends null messages" << std::endl;
                    UInt32 lookahead = 0;
                    UInt64 sendTime = GetSimulationTime();
                    UInt64 recvTime = sendTime + lookahead;
                    EventInterface *newEvent = new EventInterface();
                    newEvent->SetEventType(AppNullMessage);
                    newEvent->SetSenderReference(GetID());
                    newEvent->SetReceiverReference((*destClusters)[i]);
                    newEvent->SetSendTime(sendTime);
                    newEvent->SetReceiveTime(recvTime);
                    SendEvent(newEvent);
                }
			}
            //----------------------------------------------------
            const System::String &MonitorLogicalProcess::GetName()
            {
                return objName;
            }
			//----------------------------------------------------
			State *MonitorLogicalProcess::GetState()
			{
				return null;
				//return myState;
			}
			//----------------------------------------------------
			State *MonitorLogicalProcess::AllocateState()
			{
				return null;
				//return (State *)new ServerState();
			}
			//----------------------------------------------------
			void MonitorLogicalProcess::DeallocateState(const State *state)
			{
				//delete (ServerState *)state;
			}
			//----------------------------------------------------
			void MonitorLogicalProcess::DeallocateEvent(const EventInterface *e)
			{
                //if(e->GetEventType() == AppNullMessage)
                //    delete e;
                //else
                    delete (LogicEvent *)e;
			}
            //----------------------------------------------------
            void MonitorLogicalProcess::ShowResults()
            {
				//*logFile << "Local Time: " << GetSimulationTime() << " Frame: " << frame << " Input bits: "; ///
				/**logFile << "@time " << GetSimulationTime() << " "; ///
				for(register Int32 i = 0 ; i < numberOfInputs ; i++) ///
					*logFile << inputBits[i]; ///

				*logFile << endl; ///*/

				cout << "@time " << GetSimulationTime() << " ";\
				for(register Int32 i = 0 ; i < numberOfInputs ; i++)\
					cout << inputBits[i];\
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

