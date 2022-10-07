/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __MyApplication__
#define __MyApplication__

#include <Application.h>
#include <IntVTime.h>
#include <PartitionInfo.h>
#include <RoundRobinPartitioner.h>
#include <DeserializerManager.h>
#include <IntVTime.h>

#include "SourceObject.h"
#include "SinkObject.h"
#include "ServerObject.h"
#include "QueueEvent.h"
#include "PipelineTandemQueuePartitioner.h"

#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

/**------------------Global Settings------------**/
//const int numOfLogicalProcessors = 2;
const int simulateUntil = 100000;
const int numOfServers = 10000;
const int numOfPatterns = 1000;
const bool parallelExecution = true;

class MyApplication : public Application
{
    //----------------------------------------------------
    private: unsigned int numObjects;
    //----------------------------------------------------
    public: MyApplication();

    /// Obligatory methods to be implemented.

    public: int getNumberOfSimulationObjects(int mgrId) const;
    public: const PartitionInfo *getPartitionInfo(unsigned int numberOfProcessorsAvailable);
    public: int finalize();
    public: void registerDeserializers();
    public: const VTime &getPositiveInfinity(){return IntVTime::getIntVTimePositiveInfinity();}
    public: const VTime &getZero(){ return IntVTime::getIntVTimeZero();}
    public: const VTime& getTime(string &simulateUntil);
    //public: const VTime &getTime(string &){return IntVTime::getIntVTimeZero();}

    /// Optional methods to be implemented.
    //----------------------------------------------------

};

#endif
