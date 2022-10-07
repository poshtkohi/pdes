/**
	#define meta ...
	printf("%s\n", meta);
**/

//---------------------------------------
#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/String/String.h>
#include <System/Convert/Convert.h>
#include <System/DateTime/DateTime.h>

#include "CircuitReader.h"

#include <Parvicursor/xSim/PdesTime.h>
#include <Parvicursor/xSim/priority_queue_lc.h>

using namespace Parvicursor::PS2;
//---------------------------------------
using namespace System;

using namespace Parvicursor::xSim::Samples;
//---------------------------------------
int main(int argc, char **argv)
{
    /*PdesTime t1 = PdesTime(1, 10, 2);
    PdesTime t2 = PdesTime(1, 10, 2);
    PdesTime t3;

    std::cout << t1 << std::endl;
    std::cout << t2 << std::endl;
    if(t1 < t2)
        std::cout << "t1 < t2" << std::endl;
    else if(t1 > t2)
        std::cout << "t1 > t2" << std::endl;
    else if(t1 == t2)
        std::cout << "t1 == t2" << std::endl;

    priq_lc_test();

    return 0;*/
    UInt32 numOfLogicalProcessors = 1;

    // Verify two args were passed in
    if(argc == 2)
    {
        fprintf(stderr, "USAGE: ./GateLevelSimulation.exe <Integer value>\n");

        Int32 num = atoi(argv[1]);

        if(num <= 0)
        {
            fprintf(stderr, "USAGE: %d must be >= 1\n", num);
            exit(1);
        }
        else
        {
            numOfLogicalProcessors = num;
        }
    }

	try
	{
		CircuitReader cr = CircuitReader(numOfLogicalProcessors);
		cr.Elaborate();
		cr.StartAll();
		cr.WaitOnAllCompletion();
		//cout << "Hello World" << endl;
	}
	catch(Exception &e)
	{
		cout << "Caught exception: " << e.get_Message().get_BaseStream() << endl;
		abort();
	}
	catch(...)
	{
		cout << "Unknown exception caught" << endl;
		abort();
	}

}
//---------------------------------------
