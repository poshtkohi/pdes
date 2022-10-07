/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "DriverLogicalProcess.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            DriverLogicalProcess::DriverLogicalProcess(const System::String &objName, Int32 numberOfOutputPorts, OutputPort **outputPorts, Int32 criticalPathDelay):
                                objName(objName), numberOfOutputPorts(numberOfOutputPorts), outputPorts(outputPorts), criticalPathDelay(criticalPathDelay)
            {
            }
            //----------------------------------------------------
            DriverLogicalProcess::~DriverLogicalProcess()
            {
                if(!disposed)
                {
                    disposed = true;
                }
            }
			//----------------------------------------------------
			void DriverLogicalProcess::Initialize()
			{
				//logFile = new ofstream(("out/" + GetName() + ".txt").get_BaseStream(), ios::out); ///

				patternReader = new fstream(); ///
				patternReader->open("input.txt", ios::in); ///

				frame = 0;
				sent = 0;

				//if(!logFile->is_open())
				//	throw IOException("Could not open the file out/" + GetName() + ".txt");

				if(!patternReader->is_open())
					throw IOException("Could not open the file out/input.txt");

				endOfPatternsReached = false;

				CopyPatternToOutputPorts();

				if(endOfPatternsReached)
					return ;

                Int64 simTime = GetSimulationTime();
                Int64 sendTime = simTime;
                Int64 recvTime = simTime;

				LogicEvent *sendToSelf = new LogicEvent(sendTime, recvTime, this->GetID(), this->GetID());
				sendToSelf->sourcePort = -1;
				sendToSelf->destinationPort = -1; //will be set in ExecuteProcess();
				sendToSelf->frame = frame;

				SendEvent(sendToSelf);

				/*for(register Int32 i = 0 ; i < numberOfOutputPorts ; i++)
				{
					OutputPort *port = outputPorts[i];
					LogicEvent *newEvent = new LogicEvent(sendTime, recvTime, this, this);
					newEvent->bitValue = port->bitValue;
					newEvent->sourcePort = port->portId;
					newEvent->destinationPort = 0; //will be set in ExecuteProcess();
					newEvent->frame = frame;
					SendEvent(newEvent);
				}*/
			}
			//----------------------------------------------------
			void DriverLogicalProcess::Finalize()
			{
				//logFile->flush(); ///
				//logFile->close(); ///
				//delete logFile; ///
				patternReader->close(); ///
				delete patternReader; ///
			}
			//----------------------------------------------------
			void DriverLogicalProcess::ExecuteProcess()
			{
				//cout << "DriverLogicalProcess::ExecuteProcess() 1" << endl;

				LogicEvent *logicEvent = (LogicEvent *)GetNextEvent();
				Int32 count = 0;
                count++;

                //cout << "DriverLogicalProcess::ExecuteProcess() 2 " << logicEvent << endl;

                if(logicEvent != null)
                {
                    //*logFile << "\tAt LVT: " << GetSimulationTime() << " Frame: " << logicEvent->frame << " Input bit: " << logicEvent->bitValue << " Port: " << logicEvent->sourcePort << endl; ///
                    //cout << GetName().get_BaseStream() << " At LVT: " << GetSimulationTime() << " Frame: " << logicEvent->frame << " Input bit: " << logicEvent->bitValue << " Port: " << logicEvent->sourcePort << endl; ///
                    /*OutputPort *port = outputPorts[logicEvent->sourcePort];
                    sent++;

                    //send the event to the gates
					if(logicEvent->bitValue >= 0)
					{
						Int64 simTime = GetSimulationTime();
						Int64 sendTime = simTime;
						Int64 recvTime = simTime;

						for(register Int32 i = 0 ; i < port->numberOfOutputGates ; i++)
						{
							LogicEvent *sendToGate = new LogicEvent(sendTime, recvTime, this, GetLogicalProcessById( port->destGateLogicalProcesses[i] ));
							sendToGate->bitValue = logicEvent->bitValue;
							sendToGate->sourcePort = logicEvent->sourcePort;
							sendToGate->destinationPort = port->desPortIds[i];
							sendToGate->frame = logicEvent->frame;

							SendEvent(sendToGate);
							///cout << (*port->desPortIds)[i] << " receives the event" << endl;
							///cout << "des port is " << (*port->desPortIds)[i] <<endl;
						}
					}*/

					Int64 simTime = GetSimulationTime();
					Int64 sendTime = simTime;
					Int64 recvTime = simTime + 1;

					for(register Int32 i = 0 ; i < numberOfOutputPorts ; i++)
					{
						OutputPort *port = outputPorts[i];
						if(port->lastBitValue == port->bitValue)
							continue;
						for(register Int32 j = 0 ; j < port->numberOfOutputGates ; j++)
						{
							LogicEvent *sendToGate = new LogicEvent(sendTime, recvTime, this->GetID(), port->destGateLogicalProcesses[j] );
							sendToGate->bitValue = port->bitValue;//logicEvent->bitValue;
							sendToGate->sourcePort = logicEvent->sourcePort;
							sendToGate->destinationPort = port->desPortIds[j];
							sendToGate->frame = logicEvent->frame;

							SendEvent(sendToGate);
						}
					}
                    //cout << "GetSimulationTime(): " << GetSimulationTime() << " bitValue: " << logicEvent->bitValue << endl;
                    //delete logicEvent;
				}


				/*if(sent == numberOfOutputPorts && !endOfPatternsReached)
				{
					sent = 0;
					frame++;
					CopyPatternToOutputPorts();
					if(endOfPatternsReached)
						return;

                    Int64 simTime = GetSimulationTime();
                    Int64 sendTime = simTime;
                    Int64 recvTime = simTime + 1;

					for(register Int32 i = 0 ; i < numberOfOutputPorts ; i++)
					{
						OutputPort *port = outputPorts[i];
						if(port->lastBitValue == port->bitValue)
							continue;
						LogicEvent *sendToSelf = new LogicEvent(sendTime, recvTime, this, this);
						if(port->lastBitValue == port->bitValue)
							sendToSelf->bitValue = -1;
						else
							sendToSelf->bitValue = port->bitValue;
						sendToSelf->sourcePort = port->portId;
						sendToSelf->destinationPort = 0; //will be set in ExecuteProcess();
						sendToSelf->frame = frame;
						SendEvent(sendToSelf);
					}
				}*/

				if(!endOfPatternsReached)
				{
					frame++;
					CopyPatternToOutputPorts();
					if(endOfPatternsReached)
						return;

                    Int64 simTime = GetSimulationTime();
                    Int64 sendTime = simTime;
                    Int64 recvTime = simTime + 5;// + 1

					LogicEvent *sendToSelf = new LogicEvent(sendTime, recvTime, this->GetID(), this->GetID());
					sendToSelf->sourcePort = -1;
					sendToSelf->destinationPort = -1; //will be set in ExecuteProcess();
					sendToSelf->frame = frame;

					SendEvent(sendToSelf);
				}

				//cout << "---------- count: " << count << endl;

			}
			//----------------------------------------------------
			const System::String &DriverLogicalProcess::GetName()
			{
				return objName;
			}
			//----------------------------------------------------
			State *DriverLogicalProcess::GetState()
			{
				return null;
				//return myState;
			}
			//----------------------------------------------------
			State *DriverLogicalProcess::AllocateState()
			{
				return null;
				//return (State *)new ServerState();
			}
			//----------------------------------------------------
			void DriverLogicalProcess::DeallocateState(const State *state)
			{
				//delete (ServerState *)state;
			}
			//----------------------------------------------------
			void DriverLogicalProcess::DeallocateEvent(const EventInterface *e)
			{
				delete (LogicEvent *)e;
			}
            //----------------------------------------------------
            Int32 DriverLogicalProcess::GetBinaryValue(char c)
            {
                if(c == '0')
                    return 0;
                if(c == '1')
                    return 1;
                else
                    return -1;
            }
            //----------------------------------------------------
            void DriverLogicalProcess::CopyPatternToOutputPorts()
            {
				std::string line;
				if(!getline(*patternReader, line))
				{
					endOfPatternsReached = true;
					return;
					//patternReader->clear();
					//patternReader->seekg(0, ios::beg);
					//getline(*patternReader, line);
				}

				for(register Int32 i = 0 ; i < numberOfOutputPorts ; i++)
				{
					outputPorts[i]->lastBitValue = outputPorts[i]->bitValue;
					outputPorts[i]->bitValue = GetBinaryValue(line.c_str()[i]);
				}

				//cout << "Pattern in Frame " << frame << ": " << line << endl;
				///cout << "pattern in original frame " << frame << ": " << s << endl;

				//*logFile << "Local Time: " << GetSimulationTime() << " Frame: " << frame << " Input bits: " << line; ///

				//*logFile << endl; ///
            }
            //----------------------------------------------------
            void DriverLogicalProcess::PrintInformation()
            {
				cout << "------------------------------------------------\n";
				cout << "The logical process '" << GetName().get_BaseStream() << "' was successfully instantiated.\n";
				cout << "Information of Logical Process '" << GetName().get_BaseStream() << "':\n";
				cout << "GetID: " << GetID() << "\n";
				cout << "numberOfOutputPorts: " << numberOfOutputPorts << "\n";
				cout << "criticalPathDelay: " << criticalPathDelay << "\n";
				cout << "------------------------------------------------";
				cout << endl;
            }
            //----------------------------------------------------
            void DriverLogicalProcess::SendTerminationEvent()
            {
            }
            //----------------------------------------------------
            OutputPort **DriverLogicalProcess::GetOutputPorts(Int32 &size)
            {
				size = numberOfOutputPorts;
				return outputPorts;
            }
            //----------------------------------------------------
            Int32 DriverLogicalProcess::GetObjectDelay()
            {
                return 0;
            }
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
