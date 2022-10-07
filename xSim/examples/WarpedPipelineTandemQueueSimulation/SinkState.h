/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __SinkState__
#define __SinkState__

#include <State.h>
//#include <FlatState.h>
#include <sstream>

using namespace std;

class SinkState : public State//public FlatState
{
    //----------------------------------------------------
    public: int numEventProcessed;
    //----------------------------------------------------
    public: SinkState()
    {
        //numValue = 0;
        numEventProcessed = 0;
    }
    //----------------------------------------------------
    public: ~SinkState()
    {
    }
    //----------------------------------------------------
    public: void copyState(const State *toCopy)
    {
        //ASSERT(toCopy != 0);
        const SinkState *state = dynamic_cast<const SinkState *>(toCopy);
        numEventProcessed = state->numEventProcessed;
    }
    //----------------------------------------------------
    public: unsigned int getStateSize() const
    {
        return sizeof(SinkState);
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
