/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __SinkObject__
#define __SinkObject__

#include <warped.h>
#include <SimulationObject.h>
#include <IntVTime.h>
#include <SimulationStream.h>

#include "SinkState.h"
#include "QueueEvent.h"
//#include <stdlib.h>
//#define NULL 0

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

using std::string;
using std::vector;

using namespace std;


class SinkObject : public SimulationObject
{
//----------------------------------------------------
    // The name of this object.
    private: string objName;
    // set of destination object handles
    private: SimulationObject **outputHandles;
    // vector of destination object names
    private: vector<string> *outputNames;
    private: int numOfInputs;
    private: int criticalDelayPath;
    private: int numOfPatterns;
    private: int numberOfOutputs;
    private: int objectDelay;
    // For log operations.
    //private: ofstream logFile;
    private: SimulationStream *log;
    private: bool disposed;
    //----------------------------------------------------
    public: SinkObject(string &objName, int numOfInputs, int criticalDelayPath, int numOfPatterns, int numberOfOutputs, int objectDelay, vector<string> *outputNames);
    public: ~SinkObject();

    /// Obligatory methods to be implemented.

    public: void initialize();
    public: void finalize();
    public: void executeProcess();
    public: State *allocateState();
    public: void deallocateState( const State* state );
    public: void reclaimEvent(const Event *event);
    public: const string &getName() const { return objName; }

    /// Optional methods to be implemented.
    public: void PrintInformation();
    //----------------------------------------------------

};

#endif
