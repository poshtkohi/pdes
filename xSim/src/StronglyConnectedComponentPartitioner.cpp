/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "StronglyConnectedComponentPartitioner.h"

//**************************************************************************************************************//
// http://www.geeksforgeeks.org/tarjan-algorithm-find-strongly-connected-components/
// A C++ program to find strongly connected components in a given
// directed graph using Tarjan's algorithm (single DFS)
#include<iostream>
#include <list>
#include <stack>
#define NIL -1
using namespace std;

// A class that represents an directed graph
class Graph
{
    int V;    // No. of vertices
    std::list<int> *adj;    // A dynamic array of adjacency lists
    std::vector<std::vector<int>* > scc;    // A dynamic array of adjacency lists


    // A Recursive DFS based function used by SCC()
    void SCCUtil(int u, int disc[], int low[],
                 stack<int> *st, bool stackMember[]);
public:
    Graph(int V);   // Constructor
    ~Graph();   // Destrtuctor
    void addEdge(int v, int w);   // function to add an edge to graph
    const std::vector<std::vector<int>* > *SCC();    // finds strongly connected components
    void printSCCs();    // prints strongly connected components
};

Graph::Graph(int V)
{
    this->V = V;
    adj = new std::list<int>[V];
}

Graph::~Graph()
{
    adj->clear();
    delete[] adj;

    for(int i = 0 ; i < scc.size() ; i++)
        delete scc[i];

    scc.clear();
}

void Graph::addEdge(int v, int w)
{
    adj[v].push_back(w);
}

// A recursive function that finds and prints strongly connected
// components using DFS traversal
// u --> The vertex to be visited next
// disc[] --> Stores discovery times of visited vertices
// low[] -- >> earliest visited vertex (the vertex with minimum
//             discovery time) that can be reached from subtree
//             rooted with current vertex
// *st -- >> To store all the connected ancestors (could be part
//           of SCC)
// stackMember[] --> bit/index array for faster check whether
//                  a node is in stack
void Graph::SCCUtil(int u, int disc[], int low[], stack<int> *st,
                    bool stackMember[])
{
    // A static variable is used for simplicity, we can avoid use
    // of static variable by passing a pointer.
    static int time = 0;

    // Initialize discovery time and low value
    disc[u] = low[u] = ++time;
    st->push(u);
    stackMember[u] = true;

    // Go through all vertices adjacent to this
    list<int>::iterator i;
    for (i = adj[u].begin(); i != adj[u].end(); ++i)
    {
        int v = *i;  // v is current adjacent of 'u'

        // If v is not visited yet, then recur for it
        if (disc[v] == -1)
        {
            SCCUtil(v, disc, low, st, stackMember);

            // Check if the subtree rooted with 'v' has a
            // connection to one of the ancestors of 'u'
            // Case 1 (per above discussion on Disc and Low value)
            low[u]  = min(low[u], low[v]);
        }

        // Update low value of 'u' only of 'v' is still in stack
        // (i.e. it's a back edge, not cross edge).
        // Case 2 (per above discussion on Disc and Low value)
        else if (stackMember[v] == true)
            low[u]  = min(low[u], disc[v]);
    }

    // head node found, pop the stack and print an SCC
    int w = 0;  // To store stack extracted vertices
    if (low[u] == disc[u])
    {
        std::vector<int> *new_vertex = new std::vector<int>;
        while (st->top() != u)
        {
            w = (int) st->top();
            new_vertex->push_back(w);
            //cout << w << " ";
            stackMember[w] = false;
            st->pop();
        }
        w = (int) st->top();
        new_vertex->push_back(w);
        scc.push_back(new_vertex);
        //cout << w << "\n";
        stackMember[w] = false;
        st->pop();
    }
}

// The function to do DFS traversal. It uses SCCUtil()
const std::vector<std::vector<int>* > *Graph::SCC()
{
    if(scc.size() != 0)
        return &scc;

    int *disc = new int[V];
    int *low = new int[V];
    bool *stackMember = new bool[V];
    stack<int> *st = new stack<int>();

    // Initialize disc and low, and stackMember arrays
    for (int i = 0; i < V; i++)
    {
        disc[i] = NIL;
        low[i] = NIL;
        stackMember[i] = false;
    }

    // Call the recursive helper function to find strongly
    // connected components in DFS tree with vertex 'i'
    for (int i = 0; i < V; i++)
        if (disc[i] == NIL)
            SCCUtil(i, disc, low, st, stackMember);

    delete[] disc;
    delete[] low;
    delete[] stackMember;

    return &scc;
}

void Graph::printSCCs()
{
    std::cout << "Number of SCCs is " << scc.size() << std::endl;
    for(int i = 0 ; i < scc.size() ; i++)
    {
        std::cout << "Vertices of SCC " << i << " are ";
        for(int j = 0 ; j < scc[i]->size() ; j++)
            std::cout << (*scc[i])[j] << " ";
        std::cout << std::endl;
    }
}

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		StronglyConnectedComponentPartitioner::StronglyConnectedComponentPartitioner(/*LogicalProcess *root*/) : numOfVertices(0), numOfEdges(0)
		{
			/*if(root == null)
				throw ArgumentNullException("root", "root is null");*/

			disposed = false;
		}
		//----------------------------------------------------
		StronglyConnectedComponentPartitioner::~StronglyConnectedComponentPartitioner()
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
		PartitionInfo *StronglyConnectedComponentPartitioner::Partition(Vector<LogicalProcess *> *objects, UInt32 &numberOfPartitions)
		{
			if(disposed)
				throw ObjectDisposedException("StronglyConnectedComponentPartitioner", "The StronglyConnectedComponentPartitioner has been disposed");

			if(objects == null)
				throw ArgumentNullException("objects", "objects is null");

			if(numberOfPartitions <= 0)
				throw ArgumentOutOfRangeException("numberOfPartitions", "numberOfPartitions is less than or equal to zero");

			BuildGraph(objects);
			std::cout << "numOfVertices " << numOfVertices << std::endl;

			//DFSUtil(vertices[0]);

            GenerateDotGraph("scc-main.dot", vertices);

            //PrintSCCs();

            Graph g(numOfVertices);

            for(register Int32 i = 0 ; i < vertices.Size() ; i++)
            {
                Vertex *u = vertices[i];
                for(std::map<Vertex *, Vertex *>::iterator it = u->adjList.begin() ; it != u->adjList.end() ; ++it)
                {
                    Vertex *v = it->first;
                    g.addEdge(u->owner->GetID(), v->owner->GetID());
                }
            }

            const std::vector<std::vector<int>* > *sccs = g.SCC();
            //g.printSCCs();

            UInt32 perCoreAvg = numOfVertices / numberOfPartitions;
            std::cout << "perCoreAvg " << perCoreAvg << std::endl;

            PartitionInfo *info = new PartitionInfo();//
            //Vector< Vector<LogicalProcess*>* > partitions;
            Vector<LogicalProcess*>* partition = new Vector<LogicalProcess*>;
            info->AddPartition(partition);

            for(UInt32 sccCounter = 0 ; sccCounter < sccs->size() ; sccCounter++)
            {
                std::vector<int> *scc = (*sccs)[sccCounter];

                if(partition->Size() > perCoreAvg && sccCounter != sccs->size() - 1)
                {
                    partition = new Vector<LogicalProcess*>;
                    info->AddPartition(partition);
                }

                for(UInt32 j = 0 ; j < scc->size() ; j++)
                {
                    vertices[ (*scc)[j] ]->owner->SetOwnerClusterID(info->GetNumberOfPartitions() - 1);
                    partition->PushBack( vertices[ (*scc)[j] ]->owner );
                }

                /*if(partition->Size() > perCoreAvg)
                {
                    partition = new Vector<LogicalProcess*>;
                    info->AddPartition(partition);
                }*/
            }
			//UInt32 remainder = sortedList->get_Count() % numberOfPartitions;

            numberOfPartitions = info->GetNumberOfPartitions();
            //PrintGraph();

#if defined __Parvicursor_xSim_Debug_Enable__
			info->PrintInformation();
#endif
			//exit(0);
			return info;
		}
		//----------------------------------------------------
		void StronglyConnectedComponentPartitioner::BuildGraph(Vector<LogicalProcess *> *objects)
		{
			numOfVertices = objects->Size();

			for(register UInt32 i = 0 ; i < numOfVertices ; i++)
			{
				Vertex *v = new Vertex();
				v->owner = (*objects)[i];
				v->visited = false;
				vertices.PushBack(v);
			}

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
                    if(u->adjList.count(v) == 0)\
                        u->adjList.insert(std::pair<Vertex *, Vertex *>(v, v));
                    //if(v->adjList.count(u) == 0)\
                       v->adjList.insert(std::pair<Vertex *, Vertex *>(u, u));
				}
			}

            std::cout << "StronglyConnectedComponentPartitioner::BuildGraph()" << std::endl;
		}
        //----------------------------------------------------
        void StronglyConnectedComponentPartitioner::GenerateDotGraph(const String &filename, Vector<Vertex *> &vertices)
        {
            if(vertices.Size() == 0)
                return;
            //dot -Tpdf lpGraph.dot -o lpGraph.pdf
            //dot -Tsvg lpGraph.dot -o lpGraph.svg
            std::cout << "StronglyConnectedComponentPartitioner::GenerateDotGraph() filename " << filename.get_BaseStream() << std::endl;

            ofstream *dotFile = new ofstream(filename.get_BaseStream(), ios::out);

            if(!dotFile->is_open())
            {
                delete dotFile;
                throw IOException("Could not open the file " + filename);
            }

            std::stringstream ss;
            ss << "digraph g\n{\n\tforcelabels=true;\n";
            //ss << "rankdir=LR\n";
            /// http://www.graphviz.org/doc/info/shapes.html
            ss << "\tnode [margin=0 fontcolor=blue fontsize=20 width=0.5 shape=circle style=filled];\n";

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
                    ss << "\t" << "p" << targetID << " -> " << "p" << destID << ";" << "\n";
                    //ss << "\t" << "n" << targetID << " -> " << "n" << destID << ";" << "\n";
                    //ss << "\t" << LogicalProcess::GetLogicalProcessById(targetID)->GetName().get_BaseStream() << " -> " << LogicalProcess::GetLogicalProcessById(destID)->GetName().get_BaseStream()  << ";" << "\n";
                }
            }

            ss << "}";

            *dotFile << ss.str();

            dotFile->flush();
            dotFile->close();
            delete dotFile;
        }
        //----------------------------------------------------
        void StronglyConnectedComponentPartitioner::PrintGraph()
        {
            if(vertices.Size() == 0)
                return;

            std::cout << "StronglyConnectedComponentPartitioner::PrintGraph()" << std::endl;
            std::cout << "Number of Vertices " << numOfVertices << std::endl;

            for(register Int32 i = 0 ; i < vertices.Size() ; i++)
            {
                Vertex *u = vertices[i];
                UInt32 targetID = u->owner->GetID();
                std::string targetName = u->owner->GetName().get_BaseStream();
                for(std::map<Vertex *, Vertex *>::iterator it = u->adjList.begin() ; it != u->adjList.end() ; ++it)
                {
                    Vertex *v = it->first;
                    UInt32 destID = v->owner->GetID();
                    std::cout <<  "\t" << targetID << " -> " << destID << std::endl;
                }
                std::cout << std::endl;
            }
        }
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
