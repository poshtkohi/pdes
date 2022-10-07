#include "MyApplication.h"

#include <WarpedMain.h>

#include <iostream>

using namespace std;

//---------------------------------------------------------
int main(int argc, char **argv)
{
    string configuration = "/root/devel/xParvicursor/Samples/xSim/WarpedClusteredQueuingNetworkSimulation/bin/Debug/parallel.json";
    //string configuration = "/root/devel/xParvicursor/Samples/xSim/WarpedClusteredQueuingNetworkSimulation/bin/Debug/sequential.json";
    string simulateUntil = "40";
    bool debug = false;

    //MyApplication *application = new MyApplication();
    //application->getPartitionInfo(10);

    WarpedMain wm(new MyApplication(), configuration, simulateUntil, debug);

    return wm.main(argc, argv);
    //return 0;
}
//---------------------------------------------------------
