/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __ClusteredQueuingNetworkPartitioner__
#define __ClusteredQueuingNetworkPartitioner__


#include <warped.h>
#include <Partitioner.h>
#include <PartitionInfo.h>

class ClusteredQueuingNetworkPartitioner : public Partitioner
{
    private: int numOfRows;
    private: int numOfServersPerRow;
    public: ClusteredQueuingNetworkPartitioner(int numOfRows, int numOfServersPerRow);
    public: const PartitionInfo* partition(const vector<SimulationObject*> *objects, const unsigned int numLPs) const;
};

#endif
