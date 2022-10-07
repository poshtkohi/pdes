/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "ClusteredQueuingNetworkPartitioner.h"


//----------------------------------------------------
ClusteredQueuingNetworkPartitioner::ClusteredQueuingNetworkPartitioner(int numOfRows, int numOfServersPerRow) : numOfRows(numOfRows), numOfServersPerRow(numOfServersPerRow)
{
}
//----------------------------------------------------

const PartitionInfo *ClusteredQueuingNetworkPartitioner::partition(const vector<SimulationObject*>* objects, const unsigned int numLPs) const
{
    PartitionInfo *info = new PartitionInfo(numLPs);

    vector< vector<SimulationObject*>* > partitions(numLPs);

    for(register int i = 0 ; i < numLPs ; i++)
        partitions[i] = new vector<SimulationObject *>;

    int core = 0;
    for(register int i = 0 ; i < numOfRows ; i++)
    {
        for(register int j = 0 ; j < numOfServersPerRow ; j++)
        {
            //(*objects)[i * numOfServersPerRow + j]->SetOwnerClusterID(core);
            partitions[core]->push_back((*objects)[i * numOfServersPerRow + j]);
        }
        core++;
        if(core == numLPs)
            core = 0;
    }

    // Dispatches source and sink objects into the last core.

    /// Source object.
    //(*objects)[objects->size() - 2]->SetOwnerClusterID(numLPs - 1);
    partitions[numLPs - 1]->push_back((*objects)[objects->size() - 2]);

    /// Sink object.
    //(*objects)[objects->size() - 1]->SetOwnerClusterID(numLPs - 1);
    partitions[numLPs - 1]->push_back((*objects)[objects->size() - 1]);


    for(register int i = 0 ; i < numLPs ; i++)
        info->addPartition(i, partitions[i]);

    return info;
}
//----------------------------------------------------
