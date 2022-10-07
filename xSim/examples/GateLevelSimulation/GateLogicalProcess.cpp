/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "GateLogicalProcess.h"

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
			GateLogicalProcess::GateLogicalProcess(const System::String &objName, GateTypes gateType, Int32 numberOfInputs, Int32 numberOfOutputs, Int32 gateDelay, Int32 *destPorts)
				: objName(objName), gateType(gateType), numberOfInputs(numberOfInputs), numberOfOutputs(numberOfOutputs), gateDelay(gateDelay), destPorts(destPorts)
			{
				disposed = false;
				counter = 0;
			}
			//----------------------------------------------------
			GateLogicalProcess::~GateLogicalProcess()
			{
				if(!disposed)
				{
					disposed = true;
				}
			}
			//----------------------------------------------------
			void GateLogicalProcess::Initialize()
			{
				dummy1 = dummy2 = dummy3 = 0.0;

				inputBits = new Int32[numberOfInputs];
				for(register Int32 i = 0 ; i < numberOfInputs ; i++)
					inputBits[i] =  0;

				lastOutputBit = -1;
				outputBit = -1;

				lastOutputBit = outputBit;
				outputBit = ComputeOutput();

				if(lastOutputBit != outputBit)
					UpdateOutput();

				//logFile = new ofstream(("out/" + GetName() + ".txt").get_BaseStream(), ios::out); ///

				//if(!logFile->is_open())
				//	throw IOException("Could not open the file out/" + GetName() + ".txt");
			}
			//----------------------------------------------------
			void GateLogicalProcess::Finalize()
			{
				delete inputBits;
				inputBits = null;
				//logFile->flush(); ///
				//logFile->close(); ///
				//delete logFile; ///
			}
			//----------------------------------------------------
			void GateLogicalProcess::ExecuteProcess()
			{
				LogicEvent *inputEvent = (LogicEvent *)GetNextEvent();

				if(inputEvent != null)
				{
					inputBits[inputEvent->destinationPort] = inputEvent->bitValue;
					frame = inputEvent->frame; // Checks here for correct frame for all input events.
					//*logFile << "Local Time: " << GetSimulationTime() << " Receive Time: " << inputEvent->GetReceiveTime() << " Frame: " << inputEvent->frame << " Input bit: " << inputEvent->bitValue << " Port: " << inputEvent->destinationPort << endl; ///
					//cout << GetName().get_BaseStream() << " Local Time: " << GetSimulationTime() << " Receive Time: " << inputEvent->GetReceiveTime() << " Frame: " << inputEvent->frame << " Input bit: " << inputEvent->bitValue << " Port: " << inputEvent->destinationPort << endl; ///

					lastOutputBit = outputBit;
					outputBit = ComputeOutput();

					if(lastOutputBit != outputBit)
						UpdateOutput();

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

                if(normalMessageReceived)
                {
                    lastOutputBit = outputBit;
                    outputBit = ComputeOutput();

                    if(lastOutputBit != outputBit)
                        UpdateOutput();
                    else
                        if(GetSimulationManagerType() == DeadlockDetection)
                            SendNullMessages(); // Sends null messages
                }
                else if(!normalMessageReceived && appNullMessageReceived)
                    if(GetSimulationManagerType() == DeadlockDetection)
                        SendNullMessages(); // Sends null messages

                //Terminate();*/
			}
            //----------------------------------------------------
			void GateLogicalProcess::SendNullMessages()
			{
				return;
                SendNullMessages1();

                __null_messages__[GetOwnerClusterID()]++;
                if(__null_messages__[GetOwnerClusterID()] != 400) //200
                    return;
                __null_messages__[GetOwnerClusterID()] = 0;

                Vector<Int32> *destClusters = __clusters__[GetOwnerClusterID()];
                for(register Int32 i = 0 ; i < destClusters->Size() ; i++)
                {
                    //std::cout << "Sends null messages" << std::endl;
                    UInt32 lookahead = 1;
                    Int64 sendTime = GetSimulationTime();
                    Int64 recvTime = sendTime + lookahead;
                    EventInterface *newEvent = new EventInterface();
                    newEvent->SetEventType(AppNullMessage);
                    newEvent->SetSenderReference(GetID());
                    newEvent->SetReceiverReference((*destClusters)[i]);
                    newEvent->SetSendTime(sendTime);
                    newEvent->SetReceiveTime(recvTime);
                    SendEvent(newEvent);
                }
			}
			void GateLogicalProcess::SendNullMessages1()
			{
                counter++;
                if(counter != 10) //200
                    return;
                counter = 0;

                UInt32 lookahead = 1;
                Int64 sendTime = GetSimulationTime();
                Int64 recvTime = sendTime + lookahead;

				for(register Int32 i = 0 ; i < numberOfOutputs ; i++)
				{
                    EventInterface *newEvent = new EventInterface();
                    newEvent->SetEventType(AppNullMessage);
                    newEvent->SetSenderReference(GetID());
                    newEvent->SetReceiverReference(GetOutputLpId(i));
                    newEvent->SetSendTime(sendTime);
                    newEvent->SetReceiveTime(recvTime);
                    SendEvent(newEvent);
				}
			}
			//----------------------------------------------------
			const System::String &GateLogicalProcess::GetName()
			{
				return objName;
			}
			//----------------------------------------------------
			State *GateLogicalProcess::GetState()
			{
				return null;
				//return myState;
			}
			//----------------------------------------------------
			State *GateLogicalProcess::AllocateState()
			{
				return null;
				//return (State *)new ServerState();
			}
			//----------------------------------------------------
			void GateLogicalProcess::DeallocateState(const State *state)
			{
				//delete (ServerState *)state;
			}
			//----------------------------------------------------
			void GateLogicalProcess::DeallocateEvent(const EventInterface *e)
			{
                //if(e->GetEventType() == AppNullMessage)
                //    delete e;
                //else
                    delete (LogicEvent *)e;
			}
			//----------------------------------------------------
			Int32 GateLogicalProcess::ComputeOutput()
			{
				Int32 result;

				switch(gateType)
				{
					case AND_Type:
						result = 1;
						break;
					case OR_Type:
						result = 0;
						break;
					case XOR_Type:
						result = 0;
						break;
					case NAND_Type:
						result = 1;
						break;
					case NOR_Type:
						result = 0;
						break;
					case XNOR_Type:
						result = 0;
						break;
					case INV_Type:
						result = 0;
						break;
					case BUF_Type:
						result = 0;
						break;
					default:
						return -1;
				}

				for(register Int32 i = 0 ; i < numberOfInputs ; i++)
				{
					switch(gateType)
					{
						case AND_Type:
							result = (result & inputBits[i]);
							break;
						case OR_Type:
							result = (result | inputBits[i]);
							break;
						case XOR_Type:
							result = (result ^ inputBits[i]);
							break;
						case NAND_Type:
							result = (result & inputBits[i]);
							break;
						case NOR_Type:
							result = (result | inputBits[i]);
							break;
						case XNOR_Type:
							result = (result ^ inputBits[i]);
							break;
						case INV_Type:
							result = inputBits[i];
							break;
						case BUF_Type:
							result = inputBits[i];
							break;
						default:
							return -1;
					}
				}

				/*dummy1 = result;
				dummy2 = dummy1 * 1.55008 + 2.065;

				for(register Int32 i = 0 ; i < 20000; i++)
				{
					dummy3 = dummy1 * dummy2;
				}*/

				switch(gateType)
				{
					case NAND_Type:
						return (~result) & 1;
					case NOR_Type:
						return (~result) & 1;
					case XNOR_Type:
						return (~result) & 1;
					case INV_Type:
						return (~result) & 1;
					default:
						return result;
				}

			}
			//----------------------------------------------------
			void GateLogicalProcess::UpdateOutput()
			{
				Int64 simTime = GetSimulationTime();
				Int64 sendTime = simTime;
				Int64 recvTime = simTime + 1; //+gateDelay; // + 0; // + 1

				for(register Int32 i = 0 ; i < numberOfOutputs ; i++)
				{
					LogicEvent *newEvent = new LogicEvent(sendTime, recvTime, this->GetID(), GetOutputLpId(i));
					newEvent->frame = frame;
					newEvent->bitValue = outputBit;
					newEvent->destinationPort = destPorts[i];
					newEvent->sourcePort = 0;
					SendEvent(newEvent);
				}
			}
			//----------------------------------------------------
			System::String GateLogicalProcess::GetGateType()
			{
				switch(gateType)
				{
					case AND_Type:
						return "AND_Type";
					case OR_Type:
						return "OR_Type";
					case XOR_Type:
						return "XOR_Type";
					case NOR_Type:
						return "NOR_Type";
					case NAND_Type:
						return "NAND_Type";
					case XNOR_Type:
						return "XNOR_Type";
					case INV_Type:
						return "INV_Type";
					case BUF_Type:
						return "BUF_Type";
					default:
						return "Unknown_Type";
				}
			}
			//----------------------------------------------------
			void GateLogicalProcess::PrintInformation()
			{
				cout << "------------------------------------------------\n";
				cout << "The logical process '" << GetName().get_BaseStream() << "' was successfully instantiated.\n";
				cout << "Information of Logical Process '" << GetName().get_BaseStream() << "':\n";
				cout << "GetID: " << GetID() << "\n";
				cout << "gateType: " << GetGateType().get_BaseStream() << "\n";
				cout << "gateDelay: " << gateDelay << "\n";
				cout << "numberOfOutputs: " << numberOfOutputs << "\n";
				cout << "destPortIDs: ";
				for(register Int32 i = 0 ; i < numberOfOutputs ; i++)
					cout << destPorts[i] << ", ";
				cout << endl;
			}
			//----------------------------------------------------
			Int32 GateLogicalProcess::GetObjectDelay()
			{
				return gateDelay;
			}
			//----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//

