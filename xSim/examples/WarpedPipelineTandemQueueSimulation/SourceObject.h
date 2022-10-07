/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __SourceObject__
#define __SourceObject__

#include <warped.h>
#include <SimulationObject.h>
#include <IntVTime.h>

#include "SourceState.h"
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


class SourceObject : public SimulationObject
{
//----------------------------------------------------
    // The name of this object.
    private: string objName;
    // set of destination object handles
    private: SimulationObject **outputHandles;
    // vector of destination object names
    private: vector<string> *outputNames;
    private: int criticalDelayPath;
    private: int numOfPatterns;
    private: int numberOfOutputs;
    private: int frame;
    // For log operations.
    private: ofstream logFile;
    private: bool disposed;
    //----------------------------------------------------
    public: SourceObject(string &objName, int numOfPatterns, int criticalDelayPath, int numberOfOutputs, vector<string> *outputNames);
    public: ~SourceObject();

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
