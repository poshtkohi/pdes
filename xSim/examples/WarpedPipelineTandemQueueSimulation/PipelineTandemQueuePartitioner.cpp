/**
    #define meta ...
    printf("%s\n", meta);
**/

#include "PipelineTandemQueuePartitioner.h"


//----------------------------------------------------
PipelineTandemQueuePartitioner::PipelineTandemQueuePartitioner()
{
}
//----------------------------------------------------

const PartitionInfo *PipelineTandemQueuePartitioner::partition(const vector<SimulationObject*>* objects, const unsigned int numLPs) const
{
    /*PartitionInfo *info = new PartitionInfo(numLPs);

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

    return info;*/

        int numLPs_ = numLPs;
        std::cout << "Number of Cores Used: " << numLPs_ << endl;

        PartitionInfo *info = new PartitionInfo(numLPs_);

        vector< vector<SimulationObject*>* > partitions(numLPs_);

        for(register int i = 0 ; i < numLPs_ ; i++)
            partitions[i] = new vector<SimulationObject *>;

        // In this partitioning algorithm, we assume that the source and sink objects are in the beginning and end of the objects vector.
        // In the algorithm for the pipelined tandem queue network, we allocate objects blocks by blocks to each CPU core to prevent
        // deadlock in the loop-free task graph.

        int perCore = objects->size() / numLPs_ ;
        int remainder = objects->size() % numLPs_;

        //cout << "perCore: " << perCore << " remainder: " << remainder << endl;
        //exit(0);
        // Distributes objects into cores.
        for(register int core = 0 ; core < numLPs_ ; core++)
        {
            for(register int index = core * perCore ; index < core * perCore + perCore ; index++)
            {
                //(*objects)[index]->SetOwnerClusterID(core);
                partitions[core]->push_back((*objects)[index]);
            }
        }

        // Distributes the remainder objects into the last core.
        if(remainder > 0)
        {
            for(register int index = numLPs_ * perCore ; index < objects->size() ; index++)
            {
                //(*objects)[index]->SetOwnerClusterID(numLPs_ - 1);
                partitions[numLPs_ - 1]->push_back((*objects)[index]);
            }
        }

        for(register int i = 0 ; i < numLPs_ ; i++)
            info->addPartition(i, partitions[i]);

        cout << "Goodbye "  << endl;
        //exit(0);

        return info;
}
//----------------------------------------------------
