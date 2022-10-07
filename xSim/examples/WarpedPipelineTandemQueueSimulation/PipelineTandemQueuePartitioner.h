/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __PipelineTandemQueuePartitioner__
#define __PipelineTandemQueuePartitioner__


#include <warped.h>
#include <Partitioner.h>
#include <PartitionInfo.h>

#include <iostream>
using namespace std;

#include "MyApplication.h"

class PipelineTandemQueuePartitioner : public Partitioner
{
    public: PipelineTandemQueuePartitioner();
    public: const PartitionInfo* partition(const vector<SimulationObject*> *objects, const unsigned int numLPs) const;
};

#endif
