/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "MetisPartitioner.h"

#include <System.IO/IOException/IOException.h>
using namespace System::IO;

#include <iostream>
#include <sstream>
#include <fstream>
#include <sstream>

using namespace std;
#include <metis.h>

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		MetisPartitioner::MetisPartitioner(/*LogicalProcess *root*/) : numOfVertices(0), numOfEdges(0)
		{
			/*if(root == null)
				throw ArgumentNullException("root", "root is null");*/

			disposed = false;
		}
		//----------------------------------------------------
		MetisPartitioner::~MetisPartitioner()
		{
			if(!disposed)
			{
                for(register UInt32 i = 0 ; i < vertices.Size() ; i++)
                    if(vertices[i] != null)
                        delete vertices[i];

				disposed = true;
			}
		}
		//----------------------------------------------------
		PartitionInfo *MetisPartitioner::Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions)
		{
			if(disposed)
				throw ObjectDisposedException("MetisPartitioner", "The MetisPartitioner has been disposed");

			if(objects == null)
				throw ArgumentNullException("objects", "objects is null");

			if(numberOfPartitions <= 0)
				throw ArgumentOutOfRangeException("numberOfPartitions", "numberOfPartitions is less than or equal to zero");

			BuildGraph(objects);
			std::cout << "numOfVertices " << numOfVertices << " numOfEdges " << numOfEdges << std::endl;

            // A map of METIS node number -> LogicalProcess name that is used to
            // translate the metis partition info to warped partitions.
            //std::vector<std::string> lp_names(lps.size());

            // METIS parameters
            // idx_t is a METIS typedef
            idx_t nvtxs = numOfVertices; // number of verticies
            idx_t ncon = 1; // number of constraints
            idx_t nparts = numberOfPartitions; // number of partitions
            std::vector<idx_t> xadj; // part of the edge list
            std::vector<idx_t> adjncy; // part of the edge list
            std::vector<idx_t> adjwgt; // edge weights
            idx_t edgecut = 0; // output var for the final communication volume
            std::vector<idx_t> part(objects->Size()); // output var for partition list

            xadj.push_back(0);

            for(register UInt32 i = 0 ; i < numOfVertices ; i++)
			{
				Vertex *u = vertices[i];

				for(std::map<Vertex *, Vertex *>::iterator it = u->adjList.begin() ; it != u->adjList.end() ; ++it)
				{
                    // The metis file format counts from 1, but the API counts from 0. Cool.
                    Vertex *v = it->first;
                    adjncy.push_back(v->owner->GetID());
                    //adjwgt.push_back(weight);
				}

				xadj.push_back(adjncy.size());
            }

            if( METIS_PartGraphKway(&nvtxs,         // nvtxs
                                    &ncon,          // ncon
                                    &xadj[0],       // xadj
                                    &adjncy[0],     // adjncy
                                    NULL,           // vwgt
                                    NULL,           // vsize
                                    NULL,           // adjwgt, &adjwgt[0]
                                    &nparts,        // nparts
                                    NULL,           // tpwgts
                                    NULL,           // ubvec
                                    NULL,           // options
                                    &edgecut,       // edgecut
                                    &part[0]        // part
                               ) != METIS_OK)
            {
                throw System::Exception("METIS graph partitioner failed");
            }

            // Add the metis output to partitons.

            std::map<Int32, Vector<LogicalProcess *>* > parts;
            for(Int32 i = 0 ; i < nvtxs ; i++)
            {
                if(parts.count(part[i]) == 0)
                {
                    std::pair<Int32, Vector<LogicalProcess *>* > p(part[i], new Vector<LogicalProcess *>);
                    parts.insert(p);
                }
            }

            numberOfPartitions = parts.size();

            for(Int32 i = 0 ; i < nvtxs ; i++)
            {
                ///std::cout << "Vertex " << i << " is in partition " << part[i] << std::endl;
                parts[part[i]]->PushBack(LogicalProcess::GetLogicalProcessById(i));
            }

            PartitionInfo *info = new PartitionInfo();
            UInt32 n = 0;

            for(std::map<Int32, Vector<LogicalProcess *>* >::iterator it = parts.begin() ; it != parts.end() ; ++it)
            {
                Vector<LogicalProcess *> *partition  = it->second;
                for(register UInt32 i = 0 ; i < partition->Size(); i++)
                    (*partition)[i]->SetOwnerClusterID(n);
                info->AddPartition(partition);
                n++;
            }

            std::cout << "Number of Requested Partitions " << nparts << ", Number of Assigned Partitions " << numberOfPartitions << std::endl;

#if defined __Parvicursor_xSim_Debug_Enable__
			info->PrintInformation();
#endif

            //exit(0);
			return info;
		}
		//----------------------------------------------------
		void MetisPartitioner::BuildGraph(Vector<LogicalProcess *> *objects)
		{
			numOfVertices = objects->Size();

			for(register UInt32 i = 0 ; i < numOfVertices ; i++)
			{
				Vertex *v = new Vertex();
				v->owner = (*objects)[i];
				vertices.PushBack(v);
			}

			BuildNumberOfEdges(objects);

			for(register UInt32 i = 0 ; i < numOfVertices ; i++)
			{
				Vertex *u = vertices[i];

				for(register UInt32 j = 0 ; j < u->owner->GetOutputLpCount() ; j++)
				{
                    Vertex *v = vertices[ u->owner->GetOutputLpId(j) ];

                    // For Graphviz drawing
                    //if(u->adjList.count(v) == 0 && v->adjList.count(u) == 0)\
                        u->adjList.insert(std::pair<Vertex *, Vertex *>(v, v));

                    // For METIS partitioning
                    if(u->adjList.count(v) == 0 && v->adjList.count(u) == 0)\
                        u->adjList.insert(std::pair<Vertex *, Vertex *>(v, v));
                    if(v->adjList.count(u) == 0)
                       v->adjList.insert(std::pair<Vertex *, Vertex *>(u, u));
				}
			}

            std::cout << "MetisPartitioner::BuildGraph()" << std::endl;
            GenerateDotGraph("metis.dot");
		}
		//----------------------------------------------------
		void MetisPartitioner::BuildNumberOfEdges(Vector<LogicalProcess *> *objects)
		{
            // Count the number of edges.
            for(register UInt32 i = 0 ; i < numOfVertices ; i++)
			{
				Vertex *u = vertices[i];

				for(register UInt32 j = 0 ; j < u->owner->GetOutputLpCount() ; j++)
				{
                    Vertex *v = vertices[ u->owner->GetOutputLpId(j) ];

                    if(u->adjList.count(v) == 0 && v->adjList.count(u) == 0)
                    {
                        u->adjList.insert(std::pair<Vertex *, Vertex *>(v, v));
                        numOfEdges++;
                    }
				}
			}

			// Clear the adjacency list of each vertex.
            for(register UInt32 i = 0 ; i < numOfVertices ; i++)
			{
				Vertex *u = vertices[i];
				u->adjList.clear();
			}
		}
        //----------------------------------------------------
        void MetisPartitioner::GenerateDotGraph(const String &filename)
        {
            if(vertices.Size() == 0)
                return;
            //dot -Tpdf lpGraph.dot -o lpGraph.pdf
            //dot -Tsvg lpGraph.dot -o lpGraph.svg
            std::cout << "MetisPartitioner::GenerateDotGraph()" << std::endl;

            ofstream *dotFile = new ofstream(filename.get_BaseStream(), ios::out);

            if(!dotFile->is_open())
            {
                delete dotFile;
                throw IOException("Could not open the file " + filename);
            }

            std::stringstream ss;
            ss << "graph g\n{\n\tforcelabels=true;\n";

            for(register Int32 i = 0 ; i < vertices.Size() ; i++)
            {
                Vertex *u = vertices[i];
                UInt32 targetID = u->owner->GetID();
                std::string targetName = u->owner->GetName().get_BaseStream();
                //ss << "a [label="Birth of George Washington", xlabel="See also: American Revolution"];";
                //ss << "\t" <<  "n" << targetID << " [label=\"" << targetName << "\"];" << "\n";
                for(std::map<Vertex *, Vertex *>::iterator it = u->adjList.begin() ; it != u->adjList.end() ; ++it)
                {
                    Vertex *v = it->first;
                    UInt32 destID = v->owner->GetID();
                    ss << "\t" << "n" << targetID << " -- " << "n" << destID << ";" << "\n";
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
