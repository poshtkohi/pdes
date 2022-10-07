/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "SpaceTimeGraph.h"
#include "LogicalProcess.h"

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		SpaceTimeGraph::SpaceTimeGraph() : numOfVertices(0), numOfEdges(0), generated(false), numOfAssignedLogicalProcesses(0)
		{
		}
		//----------------------------------------------------
		SpaceTimeGraph::~SpaceTimeGraph()
		{
            for(register UInt32 i = 0 ; i < vertices.size() ; i++)
                if(vertices[i] != null)
                    delete vertices[i];
		}
		//----------------------------------------------------
		void SpaceTimeGraph::AddVertex(CreatedMode mode, UInt32 generator, UInt32 sender, UInt32 receiver, Int64 time)
		{
            Vertex *u = new Vertex;
            u->sender = sender;
            u->receiver = receiver;
            u->generator = generator;
            u->time = time;
            u->id = vertices.size();
            u->mode = mode;
            u->colour = White;
            u->visited = White;
            vertices.push_back(u);
            numOfVertices++;
		}
		//----------------------------------------------------
		void SpaceTimeGraph::AddEdge(UInt32 u, UInt32 v)
		{
            Vertex *u_ = vertices[u];
            // Avoids self loops
            //for(std::vector<UInt32>::iterator it = u_->adjList.begin() ; it != u_->adjList.end() ; ++it)
            //    if(*it == v)
            //        return;
            u_->adjList.push_back(v);
            numOfEdges++;
		}
		//----------------------------------------------------
		void SpaceTimeGraph::GeneratePriorities()
		{
            if(generated || vertices.size() == 0)
                return;

            generated = true;
            TopSort();

            //sortedList.reverse();
            std::cout << "SpaceTimeGraph::GeneratePriorities()\n";
            /*for(std::list<UInt32>::iterator it = sortedList.begin() ; it != sortedList.end() ; ++it)
            {
                UInt32 eid = *it;
                Vertex *u = vertices[eid];
                LogicalProcess *lp = LogicalProcess::GetLogicalProcessById(u->generator);
                if(lp->processSpaceStateId == -1)
                {
                    lp->processSpaceStateId = eid;
                    numOfAssignedLogicalProcesses++;
                }

                if(numOfAssignedLogicalProcesses == LogicalProcess::GetNumberOfLogicalProcesses())
                    break;
                std::cout << " " << eid;
            }
            std::cout << std::endl;

            // Assign the normal process ID to the procssStateSpaceID if some processes have not yet taken them
            for(register UInt32 i = 0 ; i < LogicalProcess::GetNumberOfLogicalProcesses() ; i++)
            {
                LogicalProcess *lp = LogicalProcess::GetLogicalProcessById(i);
                if(lp->processSpaceStateId == -1)
                    lp->processSpaceStateId = lp->GetID();
                std::cout << "SpaceTimeGraph::GeneratePriorities() processId " << lp->GetID() << " processSpaceStateId " << lp->processSpaceStateId << " processName " << lp->GetName().get_BaseStream() << std::endl;
            }*/

            for(register UInt32 i = 0 ; i < vertices.size() && numOfAssignedLogicalProcesses != LogicalProcess::GetNumberOfLogicalProcesses() ; i++)
                DFSUtil(vertices[i]);

                // Assign the normal process ID to the procssStateSpaceID if some processes have not yet taken them
            for(register UInt32 i = 0 ; i < LogicalProcess::GetNumberOfLogicalProcesses() ; i++)
            {
                LogicalProcess *lp = LogicalProcess::GetLogicalProcessById(i);
                if(lp->processSpaceStateId == -1)
                    lp->processSpaceStateId = lp->GetID();
                //std::cout << "SpaceTimeGraph::GeneratePriorities() processId " << lp->GetID() << " processSpaceStateId " << lp->processSpaceStateId << " processName " << lp->GetName().get_BaseStream() << std::endl;
            }

            GenerateStgFileForParallelSimulation("stg.parallel");
            //LoadStgFileForParallelSimulation("stg.parallel");
		}
        //----------------------------------------------------
		void SpaceTimeGraph::TopSort()
		{
			//for(register UInt32 i = 0 ; i < vertices.size() ; i++)
			for(register UInt32 i = vertices.size() - 1 ; i != -1 ; --i)
			{
				Vertex *u = vertices[i];
				if(u->colour == White)
					SortFromVertex(u);
			}

			/*cout << "\nSorted list is ";
			for(register Int32 i = 0 ; i < sortedList->get_Count() ; i++)
			{
				Vertex  *v = (Vertex *)sortedList->get_Value(i);
				cout << v->owner->GetName().get_BaseStream() << ", ";
			}*/

			/*Vertex *temp = null;
			while((temp = (Vertex *)q->Dequeue()) != null)
			{
				cout << temp->owner->GetName().get_BaseStream() << ", ";
			}*/

			//cout << endl;
		}
		//----------------------------------------------------
		void SpaceTimeGraph::SortFromVertex(Vertex *u)
		{
			u->colour = Grey;
			for(register UInt32 i = 0 ; i < u->adjList.size() ; i++)
			{
				Vertex *v = vertices[u->adjList[i]];
				if(v->colour == White)
					SortFromVertex(v);
				else if(v->colour == Grey)
				{
					cout << "The graph of the model has a cycle" << endl;
					exit(0);
				}
			}

			u->colour = Black;
			//q->Enqueue(v);
			sortedList.push_back(u->id);
		}
        //----------------------------------------------------
        void SpaceTimeGraph::GenerateDotGraph(const String &filename)
        {
            if(vertices.size() == 0)
                return;
            //dot -Tpdf lpGraph.dot -o lpGraph.pdf
            //dot -Tsvg lpGraph.dot -o lpGraph.svg
            std::cout << "SpaceTimeGraph::GenerateDotGraph()" << std::endl;

            ofstream *dotFile = new ofstream(filename.get_BaseStream(), ios::out);

            if(!dotFile->is_open())
            {
                delete dotFile;
                throw IOException("Could not open the file " + filename);
            }

            std::stringstream ss;
            ss << "digraph stg\n{\n\tforcelabels=true;\n";
            ss << "\tnode [margin=0 fontcolor=blue fontsize=20 width=0.5 shape=diamond style=filled];\n";

            for(register Int32 i = 0 ; i < vertices.size() ; i++)
            {
                Vertex *u = vertices[i];
                //ss << "a [label="Birth of George Washington", xlabel="See also: American Revolution"];";
                //ss << "\t" <<  "n" << targetID << " [label=\"" << targetName << "\"];" << "\n";
                //std::cout << i << " ";
                //std::cout << "@time " << u->time << " e" << u->id << " lpSender " << LogicalProcess::GetLogicalProcessById(u->sender)->GetName().get_BaseStream();
                for(register Int32 j = 0 ; j < u->adjList.size() ; j++)
                {
                    ss << "\t" << "e" << i << " -> " << "e" << u->adjList[j] << ";" << "\n";
                    //ss << "\t" << LogicalProcess::GetLogicalProcessById(u->sender)->GetName().get_BaseStream() << i << " -> " << LogicalProcess::GetLogicalProcessById(vertices[u->adjList[j]]->receiver)->GetName().get_BaseStream() << u->adjList[j] << ";" << "\n";
                    //if(u->id != u->adjList[j])
                    //ss << "\t" << LogicalProcess::GetLogicalProcessById(u->generator)->GetName().get_BaseStream() << i << " -> " << LogicalProcess::GetLogicalProcessById(vertices[u->adjList[j]]->generator)->GetName().get_BaseStream() << u->adjList[j] << ";" << "\n";
                    //if(u->id == u->adjList[j])
                     //   std::cout << "@time " << vertices[u->adjList[j]]->time << " self loop in e" << u->id << " to e" << u->adjList[j] << " lpSender " << LogicalProcess::GetLogicalProcessById(u->sender)->GetName().get_BaseStream() << " lpReceiver " << LogicalProcess::GetLogicalProcessById(vertices[u->adjList[j]]->receiver)->GetName().get_BaseStream() << std::endl;

                    //std::cout << " to e" << u->adjList[j]  << " lpReceiver " << LogicalProcess::GetLogicalProcessById(vertices[u->adjList[j]]->receiver)->GetName().get_BaseStream();
                }

                //std::cout << std::endl;
            }

            ss << "}";

            *dotFile << ss.str();

            dotFile->flush();
            dotFile->close();
            delete dotFile;
        }
        //----------------------------------------------------
        void SpaceTimeGraph::GenerateStgFileForParallelSimulation(const String &filename)
        {
            if(!generated || vertices.size() == 0)
                return;

            std::cout << "SpaceTimeGraph::GenerateStgFileForParallelSimulation()" << std::endl;

            std::ofstream *file = new std::ofstream(filename.get_BaseStream(), ios::out);

            if(!file->is_open())
            {
                delete file;
                throw IOException("Could not open the file " + filename);
            }

            UInt32 numOfLogicalProcess = LogicalProcess::GetNumberOfLogicalProcesses();
            std::stringstream ss;
            ss << numOfLogicalProcess << "\n";

            for(register UInt32 i = 0 ; i < numOfLogicalProcess ; i++)
            {
                LogicalProcess *lp = LogicalProcess::GetLogicalProcessById(i);
                ss << lp->GetID() << " " << lp->processSpaceStateId << "\n";
            }

            *file << ss.str();

            file->flush();
            file->close();
            delete file;
        }
        //----------------------------------------------------
        void SpaceTimeGraph::LoadStgFileForParallelSimulation(const String &filename)
        {

            std::cout << "SpaceTimeGraph::LoadStgFileForParallelSimulation()" << std::endl;

            std::ifstream *reader = new std::ifstream(filename.get_BaseStream(), ios::in);

            if(!reader->is_open())
            {
                delete reader;
                throw IOException("Could not open the file " + filename);
            }

            UInt32 numOfLogicalProcesses;
            *reader >> numOfLogicalProcesses;

            if(numOfLogicalProcesses != LogicalProcess::GetNumberOfLogicalProcesses())
            {
                delete reader;
                throw ArgumentOutOfRangeException("numOfLogicalProcesses", "numOfLogicalProcesses in " + filename + " is not consistent with the number of LPs in the simulation model");
            }

            UInt32 processId;
            UInt32 processSpaceStateId;

            UInt32 i;

            for(i = 0 ; i < numOfLogicalProcesses ; i++)
            {
                *reader >> processId;
                *reader >> processSpaceStateId;

                if(processId >= numOfLogicalProcesses)
                {
                    delete reader;
                    throw ArgumentOutOfRangeException("processId", "processId in file " + filename + " is out of range of the number of LPs in the simulation model");
                }

                LogicalProcess *lp = LogicalProcess::GetLogicalProcessById(processId);
                lp->processSpaceStateId = processSpaceStateId;
            }

            //std::cout << "i " << i << " numOfLogicalProcesses " << numOfLogicalProcesses << std::endl;

            if(i != numOfLogicalProcesses)
            {
                delete reader;
                throw ArgumentOutOfRangeException("numOfLogicalProcess", "The process IDs loaded from file " + filename + " is not consistent with the number of LPs in the simulation model");
            }

            reader->close();
            delete reader;
        }
        //----------------------------------------------------
        void SpaceTimeGraph::DFSUtil(Vertex *u)
        {
            if(u->time == 0)
                return;

            u->visited = true;

            //if(u->time < 3)
            //    std::cout << "e" << u->id << " time " << u->time << " generator " << LogicalProcess::GetLogicalProcessById(u->generator)->GetName().get_BaseStream() << std::endl;

            for(register Int32 i = 0 ; i < u->adjList.size() && numOfAssignedLogicalProcesses != LogicalProcess::GetNumberOfLogicalProcesses() ; i++)
            {
                Vertex *v = vertices[u->adjList[i]];

                if(u->time == v->time)
                {
                    LogicalProcess *lpu = LogicalProcess::GetLogicalProcessById(u->generator);
                    LogicalProcess *lpv = LogicalProcess::GetLogicalProcessById(v->generator);

                    if(lpu->processSpaceStateId == -1)
                    {
                        lpu->processSpaceStateId = u->id;
                        numOfAssignedLogicalProcesses++;
                    }

                    if(lpv->processSpaceStateId == -1)
                    {
                        lpv->processSpaceStateId = v->id;
                        numOfAssignedLogicalProcesses++;
                    }
                }

                if(!v->visited && numOfAssignedLogicalProcesses != LogicalProcess::GetNumberOfLogicalProcesses())
                    DFSUtil(v);
            }
        }
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
