/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "MyApplication.h"

//----------------------------------------------------
MyApplication::MyApplication()
{
    numObjects = numOfServers + 2;
}
//----------------------------------------------------
int MyApplication::finalize()
{
    return 0;
}
//----------------------------------------------------
int MyApplication::getNumberOfSimulationObjects(int mgrId) const
{
    return numObjects;
}
//----------------------------------------------------
const PartitionInfo *MyApplication::getPartitionInfo(unsigned int numberOfProcessorsAvailable)
{
    cout << "numberOfProcessorsAvailable: "<< numberOfProcessorsAvailable << endl;
    const PartitionInfo *retval = 0;
    vector<SimulationObject *> *objects = new vector<SimulationObject *>;


    PipelineTandemQueuePartitioner *myPartitioner = new PipelineTandemQueuePartitioner();
    //Partitioner *myPartitioner = new RoundRobinPartitioner();

    /**------------------Source--------------------**/
	vector<string> *outputNames = new vector<string>;
	outputNames->push_back("server1");
    string sourceObjectName = "source";
	SourceObject *source = new SourceObject(sourceObjectName, numOfPatterns, numOfServers, 1, outputNames);
    objects->push_back(source);
    /**------------------Servers-------------------**/
	vector<ServerObject *> servers;
	for(register int i = 0 ; i < numOfServers ; i++)
	{
        if(i != numOfServers - 1)
        {
            string serverObjName = "server" + to_string(i + 1);
            vector<string> *outputNames = new vector<string>;
            outputNames->push_back("server" + to_string(i + 2));
            servers.push_back(new ServerObject(serverObjName, 1, outputNames));
		}
		else
		{
            string serverObjName = "server" + to_string(i + 1);
            vector<string> *outputNames = new vector<string>;
            outputNames->push_back("sink");
            servers.push_back(new ServerObject(serverObjName, 1, outputNames));
		}
	}

    for(register int i = 0 ; i < servers.size() ; i++)
        objects->push_back(servers[i]);
	/**------------------Sink----------------------**/
	string sinkObjectName = "sink";
    SinkObject *sink = new SinkObject(sinkObjectName);
    objects->push_back(sink);




    retval = myPartitioner->partition(objects, numberOfProcessorsAvailable);

    delete objects;

    return retval;
}
//----------------------------------------------------
void MyApplication::registerDeserializers()
{
    DeserializerManager::instance()->registerDeserializer(QueueEvent::getQueueEventDataType(), &QueueEvent::deserialize);
}
//----------------------------------------------------
const VTime &MyApplication::getTime(string &simulateUntil)
{
    int number = std::stoi(simulateUntil);
    static const IntVTime time = IntVTime(number);
    return time;
}
//----------------------------------------------------
