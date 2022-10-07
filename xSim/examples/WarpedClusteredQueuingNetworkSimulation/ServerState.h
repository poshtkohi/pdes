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
    //public: int numValue;
    private: int numOfPatterns;
    private: int numOfInputs;
    public: int *frames;
    //----------------------------------------------------
    public: ServerState(int numOfInputs, int numOfPatterns) : numOfInputs(numOfInputs), numOfPatterns(numOfPatterns)
    {
        //numValue = 0;
        numEventProcessed = 0;
        if(numOfInputs > 1)
        {
            frames = new int[numOfPatterns];
            for(register int i = 0 ; i < numOfPatterns ; i++)
               frames[i] = 0;
        }
    }
    //----------------------------------------------------
    public: ~ServerState()
    {
        if(numOfInputs > 1)
            delete frames;
    }
    //----------------------------------------------------
    public: void copyState(const State *toCopy)
    {
        //ASSERT(toCopy != 0);
        const ServerState *state = dynamic_cast<const ServerState *>(toCopy);
        numEventProcessed = state->numEventProcessed;
        if(numOfInputs > 1)
        {
            for(register int i = 0 ; i < numOfPatterns ; i++)
            {
                frames[i] = state->frames[i];
                //if(frames[i] - 1 == 0)
                //    frames[i] = frames[i] - 1;
            }
        }
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
    const string getSummaryString()
    {
        std::ostringstream oss;
        oss << "numEventProcessed " << numEventProcessed;
        return oss.str();
    }
    //----------------------------------------------------
};

#endif
