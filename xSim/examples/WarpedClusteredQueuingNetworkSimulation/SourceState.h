/**
    #define meta ...
    printf("%s\n", meta);
**/

#ifndef __SourceState__
#define __SourceState__

#include <State.h>
//#include <FlatState.h>
#include <sstream>

using namespace std;

class SourceState : public State//public FlatState
{
    //----------------------------------------------------
    public: int numEventProcessed;
    //----------------------------------------------------
    public: SourceState()
    {
        numEventProcessed = 0;
    }
    //----------------------------------------------------
    public: ~SourceState()
    {
    }
    //----------------------------------------------------
    public: void copyState(const State *toCopy)
    {
        //ASSERT(toCopy != 0);
        const SourceState *state = dynamic_cast<const SourceState *>(toCopy);
        numEventProcessed = state->numEventProcessed;
    }
    //----------------------------------------------------
    public: unsigned int getStateSize() const
    {
        return sizeof(SourceState);
    }
    //----------------------------------------------------
    /// Optional methods to be implemented.
    public: void EventProcessed()
    {
        numEventProcessed++;
    }
    //----------------------------------------------------
    const string getSummaryString()
    {
        std::ostringstream oss;
        oss << "numEventProcessed " << numEventProcessed;
        return oss.str();
    }
    //----------------------------------------------------
};

#endif
