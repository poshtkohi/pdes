/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __ServerState__
#define __ServerState__

#include <State.h>
//#include <FlatState.h>
#include <sstream>

using namespace std;

class ServerState : public State//public FlatState
{
    //----------------------------------------------------
    public: int numEventProcessed;
    //----------------------------------------------------
    public: ServerState()
    {
        //numValue = 0;
        numEventProcessed = 0;
    }
    //----------------------------------------------------
    public: ~ServerState()
    {
    }
    //----------------------------------------------------
    public: void copyState(const State *toCopy)
    {
        //ASSERT(toCopy != 0);
        const ServerState *state = dynamic_cast<const ServerState *>(toCopy);
        numEventProcessed = state->numEventProcessed;
    }
    //----------------------------------------------------
    public: unsigned int getStateSize() const
    {
        return sizeof(ServerState);
    }
    //----------------------------------------------------
    /// Optional methods to be implemented.
    public: void EventProcessed()
    {
        numEventProcessed++;
    }
    //----------------------------------------------------
    /*const string getSummaryString()
    {
        std::ostringstream oss;
        oss << "numEventProcessed " << numEventProcessed;
        return oss.str();
    }*/
    //----------------------------------------------------
};

#endif
