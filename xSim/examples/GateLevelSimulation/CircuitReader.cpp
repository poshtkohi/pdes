/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "CircuitReader.h"

//**************************************************************************************************************//
Vector<Vector<Int32> *> __clusters__;
Int32 *__null_messages__ = null;
//----------------------------------------------------
static void print_matrix(const std::string &name, Int32 *x, Int32 n)
{
    for(register Int32 i = 0 ; i < n ; i++)
    {
        for(register Int32 j = 0 ; j < n ; j++)
        {
            std::cout << name << "[" << i << "][" << j << "]=" << x[n * i + j];
            std::cout << " ";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}
//----------------------------------------------------
static void OnPartitioningCompletion_handler(Object *state)
{
    SimulationManagerInterface *mgr = (SimulationManagerInterface *)state;
    if(mgr->GetSimulationManagerType() != DeadlockDetection)
        return;

    UInt32 clusterCount = mgr->GetClusterCount();

    /*for(register UInt32 i = 0 ; i < clusterCount ; i++)
    {
        Vector<Int32> vec(clusterCount);
        for(UInt32 j = 0 ; j < clusterCount ; j++)
            vec.PushBack(-1);
        clusters.PushBack(vec);
    }*/

    Int32 *clusters = new Int32[clusterCount * clusterCount];

    for(register UInt32 i = 0 ; i < clusterCount ; i++)
    {
        for(UInt32 j = 0 ; j < clusterCount ; j++)
            clusters[i * clusterCount + j] = -1;
    }

    for(register UInt32 i = 0 ; i < mgr->GetNumberOfLogicalProcesses() ; i++)
    {
        LogicalProcess *target = mgr->GetLogicalProcessById(i);
        for(register UInt32 j = 0 ; j < target->GetOutputLpCount() ; j++)
        {
            LogicalProcess *dest = LogicalProcess::GetLogicalProcessById(target->GetOutputLpId(j));
            UInt32 targetClusterID  = target->GetOwnerClusterID();
            UInt32 destClusterID = dest->GetOwnerClusterID();
            if(targetClusterID != destClusterID)
            {
                UInt32 targetID = target->GetID();
                UInt32 destID = dest->GetID();
                if(clusters[targetClusterID * clusterCount + destClusterID] == -1)
                    clusters[targetClusterID * clusterCount + destClusterID] = destID;
            }
        }
    }
    //std::cout << "cluster " << clusters[1][1] << std::endl;

   // print_matrix("clusters", clusters, clusterCount);

        __null_messages__ = new Int32[clusterCount];

    for(register UInt32 i = 0 ; i < clusterCount ; i++)
    {
        Vector<Int32> *adj = new Vector<Int32>;
        for(register UInt32 j = 0 ; j < clusterCount ; j++)
        {
            if(clusters[i * clusterCount + j] != -1)
                adj->PushBack(clusters[i * clusterCount + j]);
        }
        __clusters__.PushBack(adj);
        __null_messages__[i] = 0;
    }

    delete clusters;

    std::cout << "OnPartitioningCompletion " << mgr->GetClusterCount() << std::endl;
}
//----------------------------------------------------
namespace Parvicursor
{
	namespace xSim
	{
        namespace Samples
        {
            //----------------------------------------------------
            CircuitReader::CircuitReader(UInt32 numOfLogicalProcessors) : _numOfLogicalProcessors(numOfLogicalProcessors)
            {
                reader = new ifstream("model.txt", ios::in);
                //reader.open("model.txt", ios::in);

                //ofstream myOutStream("model.txt", std::ios::app);

                if(!reader->is_open())
                    throw IOException("Could not open the file model.txt");

                //criticalPathDelay = lastCriticalPathDelay = 0;

                elaborated = false;
                started = false;

				partitioner = null;
				manager = null;
				parallelExecution = false;
				simulationManagerType = Sequential;

                disposed = false;
            }
            //----------------------------------------------------
            CircuitReader::~CircuitReader()
            {
                if(!disposed)
                {
                    reader->close();
                    delete reader;
                    if(elaborated)
                        DisposeLps();

					if(partitioner != null)
					{
						switch(partitionerType)
						{
							case RoundRobin:
								delete (RoundRobinPartitioner *)partitioner;
								break;
							case DepthFirstSearch:
								delete (DepthFirstSearchPartitioner *)partitioner;
								break;
							case TopologicalSort:
								delete (TopologicalSortPartitioner *)partitioner;
								break;
                            //case Metis:\
								delete (MetisPartitioner *)partitioner;\
								break;
							case Levelized:
								delete (LevelizedPartitioner *)partitioner;
								break;
							default:
								delete partitioner;
								break;
						}
					}

					if(manager != null)
					{
						switch(simulationManagerType)
						{
							case Sequential:
								delete (SequentialSimulationManager *)manager;
								break;
							case DeadlockAvoidance:
								delete (DeadlockAvoidanceSimulationManager *)manager;
								break;
							case DeadlockDetection:
								delete (DeadlockDetectionSimulationManager *)manager;
								break;
							default:
								delete manager;
								break;
						}
					}

                    for(register UInt32 i = 0 ; i < __clusters__.Size() ; i++)
                            delete __clusters__[i];
                    __clusters__.Reset();

                    if(__null_messages__ != null)
                    {
                        delete __null_messages__;
                        __null_messages__ = null;
                    }

                    disposed = true;
                }
            }
            //----------------------------------------------------
            void CircuitReader::DisposeLps()
            {
                // Iterates over the map (LPs).
                std::map<std::string, Node *>::iterator it;

                for(it = lps.begin() ; it != lps.end() ; ++it)
                {
                    std::string name = it->first;
                    Node *node = it->second;
                    if(node->type == GATE)
                    {
                        GateLogicalProcess *gateLp = (GateLogicalProcess *)node->lp;
                        Int32 outputChannelsCount = gateLp->GetOutputLpCount();
                        //for(register Int32 i = 0 ; i < outputChannelsCount ; i++)
                        //    delete (PriorityQueue *) gateLp->get_OutputChannel(i);
                        delete gateLp;
                        delete (vector<std::string> *)node->data1;
                        ///delete (PriorityQueue **)node->data2;
                    }
                    else if(node->type == MONITOR)
                    {
                        delete (MonitorLogicalProcess *)node->lp;
                    }
                    else if(node->type == DRIVER)
                    {

                        DriverLogicalProcess *driver = (DriverLogicalProcess *)node->lp;
                        Int32 size;
                        OutputPort **outputPorts = driver->GetOutputPorts(size);
                        for(register Int32 i =  0 ; i < size ; i++)
                        {
                            //delete outputPorts[i]->outputGateNames;
                            delete outputPorts[i]->desPortIds;
                            //delete outputPorts[i]->outputChannels; //
                            delete outputPorts[i]->destGateLogicalProcesses; ///
                            delete outputPorts[i];
                        }

                        if(size > 0 && outputPorts != null)
                            delete outputPorts;

                        delete driver;
                        delete (vector<std::string> *)node->data1;
                        ///delete (PriorityQueue **)node->data2;
                    }

                    delete node;
                    //cout << "Dispose(): " << name << endl;
                }

                lps.clear();
            }
            //----------------------------------------------------
            void CircuitReader::Elaborate()
            {
                if(disposed)
                    throw ObjectDisposedException("CircuitReader", "The CircuitReader has been disposed");

                if(elaborated)
                    return;

				//cout << "hello1" << endl;

                /*std::string line;

                while(getline(*reader,line))
                {
                    cout << line << '\n';
                }

                cout << endl;

                reader->clear();
                reader->seekg(0, ios::beg);*/

                Int32 totalLPs; // total LPs number in the circuit
                *reader >> inputChannelCapacity;
                *reader >> totalLPs;

                cout << "inputChannelCapacity: " << inputChannelCapacity << " totalLPs: " << totalLPs << endl;

                for(register Int32 i = 0 ; i < totalLPs ; i++)
                {
                    if(i == totalLPs - 2)
                        HandleMonitorLP();
                        //cout << "Monitor found" << endl;
                    else if(i == totalLPs - 1)
                    {
                        HandleDriverLP();
                        //reader->ignore(1, '\n');
                        //std::string dummyLine;
                        //getline(*reader, dummyLine);
						//cout << "Driver ignored" << endl;
                    }
                    else
                        HandleGateLP();
                }

                // Iterates over the map (LPs).

                std::map<std::string, Node *>::iterator it;

                for(it = lps.begin() ; it != lps.end() ; ++it)
                {
                    //std::string name = it->first;
                    Node *node = it->second;
                    if(node->type == GATE || node->type == DRIVER)
                    {
                        //cout << "node->type == GATE 1" << endl; ///

                        GateLogicalProcess *gateLp = (GateLogicalProcess *)node->lp;
                        vector<std::string> *outputObjectNames = (vector<std::string> *)node->data1;

                        //cout << "node->type == GATE 2" << endl; ///

                       //cout << "outputObjectNames->size(): " << outputObjectNames->size() << endl; ///

                        std::string outputLpName;
                        // Setups gateLp output channels
                        //PriorityQueue **gateLpOutputChannels = new PriorityQueue *[ outputObjectNames->size() ];
                        for(register Int32 i = 0 ; i < outputObjectNames->size() ; i++)
                        {
                            outputLpName = (*outputObjectNames)[i];
                            //cout << "node->type == GATE 3 outputLpName: " << outputLpName << endl; ///
                            LogicalProcess *outputLp = (LogicalProcess *)((Node *)lps[outputLpName])->lp;
                            //cout << "node->type == GATE 4 outputLp->get_ID(): " << outputLp->get_ID() << endl; ///
                            ///gateLpOutputChannels[i] = outputLp->get_InputChannel();
							gateLp->AddOutputLP(outputLp->GetID());
                            //cout << "node->type == GATE 5" << endl; ///
                        }
                        //cout << "node->type == GATE 6" << endl; ///
                        ///gateLp->set_OutputChannels( (QueueInterface **)gateLpOutputChannels, outputObjectNames->size() );
                        ///node->data2 = (Object *)gateLpOutputChannels;
                        //cout << "node->type == GATE 7" << endl; ///
                    }
                    else if(node->type == MONITOR)
                    {
                        // (MonitorLogicalProcess *)node->data1;
                        //continue;
                    }
					else
						throw ArgumentException("An invalid object type");

                    //cout << name << endl;
                }

                //HardwareLogicalProcess *driver = ((Node *)lps[driverLpName])->lp;
                //DFS(driver);
                //DFS(driverLp);
                //cout << "DFS::criticalPathDelay: " << criticalPathDelay << endl;


                elaborated = true;
            }
            //----------------------------------------------------
            void CircuitReader::HandleGateLP()
            {
                //cout << "CircuitReader::HandleGateLP()" << endl;

                std::string myObjName;    // object name
                Int32 delay;           // the delay between event receiving and event sending
                Int32 numberOfInputs;  // the number of inputs of the object
				Int32 numberOfOutputPorts; // the number of inputs of the object
                Int32 numberOfOutputGates; // the number of objects connected to the object
                std::string destObjName;  // the name of the gate which receives the object output
                Int32 desInputPort;    // pin id of the gate which receives the object output

                *reader >> myObjName >> delay >> numberOfInputs >> numberOfOutputPorts >> numberOfOutputGates;
                //cout << myObjName << " " << delay << " " << numberOfInputs << " " << numberOfOutputGates;
                vector<std::string> *outputObjectNames = new vector<std::string>;
                Int32 *destinationPorts = new Int32[numberOfOutputGates];

                for(register Int32 i = 0 ; i < numberOfOutputGates ; i++)
                {
                    *reader >> destObjName;
                    //cout << " " << destObjName;
                    outputObjectNames->push_back(destObjName);
                }
                for(register Int32 i = 0 ; i < numberOfOutputGates ; i++)
                {
                    *reader >> desInputPort;
                    //cout << " " << desInputPort;
                    destinationPorts[i] = desInputPort;
                }
                //cout << endl;

                /// const char* c_str() const;
                System::String gateName =System::String (myObjName.c_str());

                GateTypes type;
                if(gateName.IndexOf("and", 0) == 0)
                    type = AND_Type;
                else if(gateName.IndexOf("xor", 0) == 0)
                    type = XOR_Type;
                else if(gateName.IndexOf("or", 0) == 0)
                    type = OR_Type;
				else if(gateName.IndexOf("nand", 0) == 0)
					type = NAND_Type;
				else if(gateName.IndexOf("xnor", 0) == 0)
					type = XNOR_Type;
				else if(gateName.IndexOf("nor", 0) == 0)
					type = NOR_Type;
				else if(gateName.IndexOf("not", 0) == 0)
					type = INV_Type;
				else if(gateName.IndexOf("buf", 0) == 0)
					type = BUF_Type;
                else
                    throw ArgumentException("An invalid gate type was specified in the model file");

                GateLogicalProcess *gateLp = new GateLogicalProcess(gateName, type, numberOfInputs, numberOfOutputGates, delay, destinationPorts);
                ///gateLp->PrintInformation();

                Node *node = new Node();
                node->lp = gateLp;
                node->data1 = (Object *)outputObjectNames;
                node->type = GATE;
                lps[myObjName] = node;
            }
            //----------------------------------------------------
            void CircuitReader::HandleMonitorLP()
            {
                //cout << "CircuitReader::HandleMonitorLP()" << endl;

                std::string myObjName;    // object name
                Int32 numberOfMonitorInputs;  // the number of inputs of the object

                *reader >> myObjName >> numberOfMonitorInputs;

                //cout << myObjName << " " << numberOfMonitorInputs << endl;

                //cout << "HandleMonitorLP() - Name: " << myObjName << " numberOfMonitorInputs: " << numberOfMonitorInputs << endl;

                /// const char* c_str() const;
                System::String monitorObjectName = System::String (myObjName.c_str());

                MonitorLogicalProcess *monitor = new MonitorLogicalProcess(monitorObjectName, numberOfMonitorInputs);
                ///monitor->PrintInformation();

                Node *node = new Node();
                node->lp = monitor;
                node->data1 = null;
                node->type = MONITOR;
                lps[myObjName] = node;

                monitorLpName = myObjName;
                monitorLp = monitor;
            }
            //----------------------------------------------------
            void CircuitReader::HandleDriverLP()
            {
                //cout << "CircuitReader::HandleDriverLP()" << endl;

                //vector<Int32> *destinationPortIds = new vector<Int32>;

                std::string myObjName;    // object name
                Int32 criticalPathDelay;  // the critical path delay of the circuit
                Int32 numberOfDriverOutputPorts;  // the number of driver ports
                Int32 numberOfOutputGates;

                vector<std::string> *outputObjectNames = new vector<std::string>;

                *reader >> myObjName >> criticalPathDelay >> numberOfDriverOutputPorts >> numberOfOutputGates;

                //__numberOfDriverOutputGates__ = numberOfOutputGates;

                //cout << "HandleDriverLP() - Name: " << myObjName << " criticalPathDelay: " << criticalPathDelay << " numberOfDriverOutputPorts: " << numberOfDriverOutputPorts << endl;
                std::string outputLpName;

                //cout << myObjName << " " << criticalPathDelay << " " << numberOfDriverOutputPorts  << " " << numberOfOutputGates;

                for(register Int32 i = 0 ; i < numberOfOutputGates ; i++)
                {
                    *reader >> outputLpName;
                    outputObjectNames->push_back(outputLpName);
                    //cout << " " << outputLpName;
                }

                OutputPort **outputPorts = new OutputPort *[numberOfDriverOutputPorts];

                std::string portName;
                Int32 portId;

                std::string gateName;
                Int32 destPortId;

                for(register Int32 i = 0 ; i < numberOfDriverOutputPorts ; i++)
                {
                    *reader >> portName >> numberOfOutputGates >> portId;
                    outputPorts[i] = new OutputPort();
                    outputPorts[i]->numberOfOutputGates = numberOfOutputGates;
                    outputPorts[i]->portId = portId;
                    outputPorts[i]->desPortIds = new Int32[numberOfOutputGates];
                    outputPorts[i]->destGateLogicalProcesses = new Int64[numberOfOutputGates];
                    //outputPorts[i]->outputChannels = new PriorityQueue *[numberOfOutputGates];
                    //PriorityQueue **driverOutputChannels = new PriorityQueue *[numberOfOutputGates];

                    //cout << " " << portName << " "<< numberOfOutputGates << " " << portId;

                    for(register Int32 j = 0 ; j < numberOfOutputGates ; j++)
                    {
                        *reader >> gateName >> destPortId;
                        //cout << " " << gateName << " " <<  destPortId;
						//outputPorts[i]->outputGateNames->push_back(gateName);
						Node *node = lps[gateName];
                        outputPorts[i]->destGateLogicalProcesses[j] = node->lp->GetID();
						//cout << "GetID(): " << node->lp->GetID() << endl;
                        outputPorts[i]->desPortIds[j] = destPortId;

                        //cout << "HandleDriverLP() - gateName: " << gateName << " destPortId: " << destPortId << endl;
                    }
                }

                /// const char* c_str() const;
                System::String driverObjectName = System::String(myObjName.c_str());

//#if defined PatternsFromFile
                Int128 pattern_start;
                Int128 pattern_end;
                *reader >> pattern_start >> pattern_end;
                //cout << " " << pattern_start << " " << pattern_end << endl;
                DriverLogicalProcess *driver = new DriverLogicalProcess(driverObjectName, numberOfDriverOutputPorts, outputPorts, criticalPathDelay);
//#else
//                mpz_class pattern_start;
//                mpz_class pattern_end;
//
//                *reader >> pattern_start >> pattern_end;
//                //cout << " " << pattern_start << " " << pattern_end << endl;
//
//                DriverLogicalProcess *driver = new DriverLogicalProcess(driverObjectName, inputChannelCapacity, numberOfDriverOutputPorts, outputPorts, criticalPathDelay, pattern_start, pattern_end);
//#endif

                ///driver->PrintInformation();

                Node *node = new Node();
                node->lp = driver;
                node->data1 = (Object *)outputObjectNames;
                node->type = DRIVER;
                lps[myObjName] = node;

                driverLpName = myObjName;
                driverLp = driver;
            }
            //----------------------------------------------------
            void CircuitReader::StartAll()
            {
				//return;
                if(disposed)
                    throw ObjectDisposedException("CircuitReader", "The CircuitReader has been disposed");

                if(!elaborated)
                    throw InvalidOperationException("Elaborate() method must first be called");

                if(!started)
                {
                    started = true;

					/**------------------Global Settings------------**/
					//const Int32 numOfLogicalProcessors = 4; //8, 96
					const Int64 simulateUntil = 100;//9999999999999;
					const Int32 channelCapacity = 1;
					//partitionerType = DepthFirstSearch;
					//partitionerType = RoundRobin;
					//partitionerType = Levelized;
					partitionerType = TopologicalSort;
					//partitionerType = Metis;
					if(_numOfLogicalProcessors == 1)
                        simulationManagerType = Sequential;
					//simulationManagerType = DeadlockAvoidance;
					else
                        simulationManagerType = DeadlockDetection;

					//printf("hello 1\n");

					/**------------------Simulation-----------------**/
					switch(partitionerType)
					{
						case RoundRobin:
							cout << "PartitionerType: RoundRobin" << endl;
							partitioner = new RoundRobinPartitioner();
							break;
						case DepthFirstSearch:
							cout << "PartitionerType: DepthFirstSearch" << endl;
							partitioner = new DepthFirstSearchPartitioner(driverLp);
							break;
						case TopologicalSort:
							cout << "PartitionerType: TopologicalSort" << endl;
							partitioner = new TopologicalSortPartitioner();
							break;
                        //case Metis:\
							cout << "PartitionerType: Metis" << endl;\
							partitioner = new MetisPartitioner();\
							break;
						case Levelized:
							cout << "PartitionerType: Levelized" << endl;
							partitioner = new LevelizedPartitioner(driverLp);
							break;
						default:
							cout << "PartitionerType: RoundRobin" << endl;
							partitioner = new RoundRobinPartitioner();
							break;
					}

					switch(simulationManagerType)
					{
						case Sequential:
							manager = new SequentialSimulationManager(channelCapacity);
							break;
						case DeadlockAvoidance:
							manager = new DeadlockAvoidanceSimulationManager(_numOfLogicalProcessors, channelCapacity, partitioner);
							break;
						case DeadlockDetection:
							manager = new DeadlockDetectionSimulationManager(_numOfLogicalProcessors, channelCapacity, partitioner);
							break;
						default:
							manager = new SequentialSimulationManager(channelCapacity);
							break;
					}

					manager->RegisterOnPartitioningCompletionCallback(OnPartitioningCompletion_handler, manager);

                    // Iterates over the map (LPs).
                    std::map<std::string, Node *>::iterator it;

                    for(it = lps.begin() ; it != lps.end() ; ++it)
                    {
                        //std::string name = it->first;
                        Node *node = it->second;
                        LogicalProcess *lp = (LogicalProcess *)node->lp;
                        manager->RegisterLogicalProcess(lp);
                    }

					manager->Run(simulateUntil);
					manager->WaitOnManagerCompletion();

                }
                else
                    throw ThreadStateException("The LPs have already been started");

            }
            //----------------------------------------------------
            void CircuitReader::WaitOnAllCompletion()
            {
                if(disposed)
                    throw ObjectDisposedException("CircuitReader", "The CircuitReader has been disposed");

                if(!elaborated)
                    throw InvalidOperationException("Elaborate() method must first be called");

                if(!started)
                    throw InvalidOperationException("StartAll() method must first be called");

                // Iterates over the map (LPs).
                /*std::map<std::string, Node *>::iterator it;

                for(it = lps.begin() ; it != lps.end() ; ++it)
                {
                    //std::string name = it->first;
                    Node *node = it->second;
                    LogicalProcess *lp = (LogicalProcess *)node->lp;
                    lp->WaitOnLogicalProcessCompletion();
                }*/
            }
            //----------------------------------------------------
            /*void CircuitReader::DFS(HardwareLogicalProcess *v)
            {
                Int32 adjacentEdgesCount = v->get_OutputChannelsCount();

                cout << "CircuitReader::DFS " << v->get_Name().get_BaseStream() << endl;

                if(v == monitorLp)
                {
                    //MonitorLogicalProcess *monitor = (MonitorLogicalProcess *)v;
                    //lastCriticalPathDelay += monitor->get_ObjectDelay();

                    //cout << "CircuitReader::DFS::monitorLp lastCriticalPathDelay: " << lastCriticalPathDelay << " criticalPathDelay: " << criticalPathDelay << endl;

                    /// Compuates the new maximum critical path delay based on the last grage traversal.
                    //if(lastCriticalPathDelay > criticalPathDelay)
                    //    criticalPathDelay = lastCriticalPathDelay;

                    //lastCriticalPathDelay = 0;
                }
                else if(v == driverLp)
                {
                    //DriverLogicalProcess *driver = (DriverLogicalProcess *)v;
                    //lastCriticalPathDelay += driver->get_ObjectDelay();
                }
                else
                {
                    GateLogicalProcess *gateLp = (GateLogicalProcess *)v;
                    gateLp->currentCriticalPathDelay += gateLp->get_ObjectDelay();
                    if(gateLp->currentCriticalPathDelay > criticalPathDelay)
                        criticalPathDelay = gateLp->currentCriticalPathDelay;
                    cout << gateLp->get_Name().get_BaseStream() << " gateLp->currentCriticalPathDelay: " << gateLp->currentCriticalPathDelay << endl;
                }

                for(register Int32 i = 0 ; i < adjacentEdgesCount ; i++)
                {
                    PriorityQueue *outputChannel = (PriorityQueue *)v->get_OutputChannel(i);
                    HardwareLogicalProcess *w= (HardwareLogicalProcess *)outputChannel->get_OwnerLogicalProcessor();
                    DFS(w);
                }
            }*/
            //----------------------------------------------------
		}
	}
};
//**************************************************************************************************************//
