/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "MyApplication.h"

//----------------------------------------------------
MyApplication::MyApplication()
{
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
    const PartitionInfo *retval = 0;
    vector<SimulationObject *> *objects = new vector<SimulationObject *>;

	/**------------------Global Settings------------**/
	const int numOfLogicalProcessors = 4;
	const int simulateUntil = 5000; // 6000 // 100
	const int numOfRows = 2;//16; //16
	const int numOfServersPerRow = 2;//200;//4, 100
	const int numOfPatterns = 10;
	const bool parallelExecution = true;
	// Each server and object delay in this example is one.
    ClusteredQueuingNetworkPartitioner *myPartitioner = new ClusteredQueuingNetworkPartitioner(numOfRows, numOfServersPerRow);
	/**------------------Rows----------------------**/
	vector<ServerObject *> servers;
	// Creates servers in each row.
	for(register int i = 0 ; i < numOfRows ; i++)
	{
		for(register int j = 0 ; j < numOfServersPerRow ; j++)
		{
			string serverObjName = "server(" + to_string(i + 1) + ", " + to_string(j + 1) + ")";
			vector<string> *outputNames = new vector<string>;

			if(j == 0)
			{
                outputNames->push_back("server(" + to_string(i + 1) + ", " + to_string(j + 1 + 1) + ")");
                servers.push_back(new ServerObject(serverObjName, 2, numOfServersPerRow, numOfPatterns, 1, 1, outputNames));
			}
			else if(j == numOfServersPerRow - 1)
			{
                outputNames->push_back("server(" + to_string(i + 1) + ", " + to_string(1) + ")");
                outputNames->push_back("sink");
                servers.push_back(new ServerObject(serverObjName, 1, numOfServersPerRow, numOfPatterns, 2, 1, outputNames));
			}
			else
			{
                outputNames->push_back("server(" + to_string(i + 1) + ", " + to_string(j + 1 + 1) + ")");
                servers.push_back(new ServerObject(serverObjName, 1, numOfServersPerRow, numOfPatterns, 1, 1, outputNames));
			}
			//objects->push_back(servers[i * numOfServersPerRow + j]);
			//cout << serverObjName << endl;
		}
	}

    for(register int i = 0 ; i < servers.size() ; i++)
        objects->push_back(servers[i]);

    cout << "objects->size(): " << objects->size() << endl ;


	/**------------------Source--------------------**/
	vector<string> *outputNames = new vector<string>;
	for(register int i = 0 ; i < numOfRows ; i++)
        outputNames->push_back("server(" + to_string(i + 1) + ", " + to_string(1) + ")");

    string sourceObjectName = "source";
	SourceObject *source = new SourceObject(sourceObjectName, 1, numOfServersPerRow + 1, numOfPatterns, numOfRows, 1, outputNames);
	/**------------------Sink----------------------**/
	outputNames = new vector<string>;
	outputNames->push_back("source");
	string sinkObjectName = "sink";
    SinkObject *sink = new SinkObject(sinkObjectName, numOfRows, numOfServersPerRow + 1, numOfPatterns, 1, 1, outputNames);

    objects->push_back(source);
    objects->push_back(sink);

    // Prints the information of all servers in each row.
	/*source->PrintInformation();
	sink->PrintInformation();
	for(register int i = 0 ; i < numOfRows ; i++)
		for(register int j = 0 ; j < numOfServersPerRow ; j++)
			servers[i * numOfServersPerRow + j]->PrintInformation();*/

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
