/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "SimulationManagerInterface.h"
#include "LogicalProcess.h"

#include <System.IO/IOException/IOException.h>
using namespace System::IO;

#include <iostream>
#include <sstream>
#include <fstream>
#include <sstream>

using namespace std;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
        void SimulationManagerInterface::GenerateLpDotGraph(const String &filename)
        {
            if(logicalProcesses->Size() == 0)
                return;
            //dot -Tpdf lpGraph.dot -o lpGraph.pdf
            //dot -Tsvg lpGraph.dot -o lpGraph.svg
            std::cout << "GenerateLpDotGraph()" << std::endl;

            ofstream *dotFile = new ofstream(filename.get_BaseStream(), ios::out); ///

            if(!dotFile->is_open())
            {
                delete dotFile;
                throw IOException("Could not open the file " + filename);
            }

            std::stringstream ss;
            ss << "digraph g\n{\n\tforcelabels=true;\n";

            for(register Int32 i = 0 ; i < logicalProcesses->Size() ; i++)
            {
                LogicalProcess *target = (*logicalProcesses)[i];
                UInt32 targetID = target->GetID();
                std::string targetName = target->GetName().get_BaseStream();
                //ss << "a [label="Birth of George Washington", xlabel="See also: American Revolution"];";
                ss << "\t" <<  "n" << targetID << " [label=\"" << targetName << "\"];" << "\n";

                for(register Int32 j = 0 ; j < target->GetOutputLpCount() ; j++)
                {
                    UInt32 destID = target->GetOutputLpId(j);
                    ss << "\t" << "n" << targetID << " -> " << "n" << destID << ";" << "\n";
                }
            }

            ss << "}";

            *dotFile << ss.str();

            dotFile->flush();
            dotFile->close();
            delete dotFile;
        }
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
