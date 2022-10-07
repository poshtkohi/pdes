/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "PdesTime.h"
#include "Vector.h"
#include "LogicalProcess.h"

//class LogicalProcess;
extern Vector<LogicalProcess *> *logicalProcesses;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
        UInt32 PdesTime::GetOwnerClusterID(UInt32 processID)
        {
            return (*logicalProcesses)[processID]->GetOwnerClusterID();
        }
        //----------------------------------------------------
        /*bool PdesTime::operator < (const PdesTime &t) const
        {
            if(_time < t._time)
                return true;
            if(_time == t._time && _lc < t._lc)
                return true;
            if(_time == t._time && _lc == t._lc && _id < t._id)
            {
                //std::cout << "this " << *this << " t " << t << std::endl;
                return true;
            }
            //if(_time == t._time && _id < t._id)
            //    return true;
            //if(_time == t._time && GetOwnerClusterID(_id) == GetOwnerClusterID(t._id) && _lc < t._lc)
            //    return true;
            return false;
        }*/
		//----------------------------------------------------
	}
};
//**************************************************************************************************************//
