/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "GateLogicalProcess.h"


//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            GateLogicalProcess::GateLogicalProcess(const String &objName, GateTypes gateType, Int32 numberOfInputs, Int32 numberOfOutputs, Int32 gateDelay, Int32 *destPorts)
                                                    : objName(objName), gateType(gateType), numberOfInputs(numberOfInputs), numberOfOutputs(numberOfOutputs), gateDelay(gateDelay), destPorts(destPorts)
            {
                disposed = false;
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
				inputBits = new Int32[numberOfInputs];
				for(register Int32 i = 0 ; i < numberOfInputs ; i++)
					inputBits[i] =  -1 - i;

				numEventProcessed = 0;
				numInputValue = 0;
				//lastOutputBit = 0;
				outputBit = -1;

				logFile = new ofstream(("out/" + GetName() + ".txt").get_BaseStream(), ios::out); ///

				if(!logFile->is_open())
					throw IOException("Could not open the file out/" + GetName() + ".txt");
			}
			//----------------------------------------------------
			void GateLogicalProcess::Finalize()
			{
				delete inputBits; 
				inputBits = null;
				logFile->flush(); ///
				logFile->close(); ///
				delete logFile; ///
			}
            //----------------------------------------------------
            void GateLogicalProcess::ExecuteProcess()
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
							lastOutputBit = outputBit;
							outputBit = ComputeOutput();
							UpdateOutput();
							numInputValue = 0;

							/*for(register Int32 i = 0 ; i < numberOfInputs ; i++)
								inputBits[i] = -1;*/
						}
					}
				}
            }
            //----------------------------------------------------
            String GateLogicalProcess::GetName()
            {
                return objName;
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
                        default:
                            return -1;
                    }
                }

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
				for(register Int32 i = 0 ; i < numberOfOutputs ; i++)
				{
					Int64 simTime = GetSimulationTime();
					Int64 sendTime = simTime;
					Int64 recvTime = simTime + gateDelay;

					LogicEvent *newEvent = new LogicEvent(sendTime, recvTime, this, GetLogicalProcessById(GetOutputLpId(i)));
					newEvent->frame = frame;
					newEvent->bitValue = outputBit;
					newEvent->destinationPort = destPorts[i];
					newEvent->sourcePort = 0;
					SendEvent(newEvent);
				}
				return ;
				/*if(outputBit == lastOutputBit)
				{
					//cout << "outputBit == lastOutputBit" << endl;
					for(register Int32 i = 0 ; i < numberOfOutputs ; i++)
					{
						Int64 simTime = GetSimulationTime();
						Int64 sendTime = simTime;
						Int64 recvTime = simTime + gateDelay;

						LogicalProcess *destLP = GetLogicalProcessById(GetOutputLpId(i));
						if((Int32)destLP->GetData() != 0)
						{
							MonitorLogicalProcess *monitorLp = (MonitorLogicalProcess *)destLP;

							if(monitorLp->numInputValue != monitorLp->numberOfInputs)
								monitorLp->numInputValue++;

							monitorLp->SetSimulationTime(recvTime);
							monitorLp->frame = frame;
							if(monitorLp->numInputValue == monitorLp->numberOfInputs)
							{
								//monitorLp->frame = frame;
								monitorLp->ShowResults();
								monitorLp->numInputValue = 0;
							}
						}
						else
						{
							GateLogicalProcess *gateLp = (GateLogicalProcess *)destLP;

							if(gateLp->numInputValue != gateLp->numberOfInputs)
								gateLp->numInputValue++;

							gateLp->SetSimulationTime(recvTime);
							gateLp->frame = frame;
							if(gateLp->numInputValue == gateLp->numberOfInputs)
							{
								gateLp->lastOutputBit = gateLp->outputBit;
								gateLp->outputBit = gateLp->ComputeOutput();
								gateLp->UpdateOutput();
								gateLp->numInputValue = 0;
							}
						}
					}
				}
				else
				{
					for(register Int32 i = 0 ; i < numberOfOutputs ; i++)
					{
						Int64 simTime = GetSimulationTime();
						Int64 sendTime = simTime;
						Int64 recvTime = simTime + gateDelay;

						LogicEvent *newEvent = new LogicEvent(sendTime, recvTime, this, GetLogicalProcessById(GetOutputLpId(i)));
						newEvent->frame = frame;
						newEvent->bitValue = outputBit;
						newEvent->destinationPort = destPorts[i];
						newEvent->sourcePort = 0;
						SendEvent(newEvent);
					}
				}*/
            }
            //----------------------------------------------------
            String GateLogicalProcess::GetGateType()
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
            /*void GateLogicalProcess::SendTerminationEvent()
            {
            }*/
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
