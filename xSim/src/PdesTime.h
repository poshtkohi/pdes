/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/


#ifndef __Parvicursor_xSim_PdesTime_h__
#define __Parvicursor_xSim_PdesTime_h__

#include <stdlib.h>

#include <general.h>
#include <System/BasicTypes/BasicTypes.h>
#include <System/Object/Object.h>
#include <System/String/String.h>

#include "Vector.h"
//#include "priority_queue_lc.h"
//#include "LogicalProcess.h"

using namespace System;
using namespace Parvicursor::xSim::psc;

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
        //----------------------------------------------------
		// forward declarations
		class DeadlockDetectionSimulationManager;
		//struct pri_queue;

        class PdesTime : public Object
        {
            /*---------------------fields-----------------*/
            friend class DeadlockDetectionSimulationManager;
            private: Int64 _time;
            private: UInt64 _lc;
            //private: UInt32 _id;
            /*---------------------methods----------------*/
            // Constructor
			public: inline PdesTime(Int64 time, UInt64 lc/*, UInt32 id*/) : _time(time), _lc(lc)//, _id(id)
			{
			}
            public: inline PdesTime() : _time(-1), _lc(0)//, _id(0)
			{
			}
            public: inline PdesTime(Int64 time) : _time(time), _lc(0)//, _id(0)
			{
			}
            //----------------------------------------------------
            // Copy Constructor
            public: inline PdesTime(const PdesTime &t)
			{
                _time = t._time;
                _lc = t._lc;
                //_id = t._id;
			}
			//----------------------------------------------------
            // assignment operators
			public: inline PdesTime& operator = (const PdesTime &t)
			{
                _time = t._time;
                _lc = t._lc;
                //_id = t._id;
				return *this;
			}
            //----------------------------------------------------
            // comparsion operators
            private: inline static UInt32 GetOwnerClusterID(UInt32 processID);
            //public: bool operator < (const PdesTime &t) const;
            public: inline bool operator < (PdesTime &t) const
            {
                if(_time < t._time)
                    return true;
                else if(_time == t._time && _lc < t._lc)
                    return true;
                //else if(_time == t._time && _lc == t._lc && _id < t._id)
                //    return true;
                else
                    return false;
            }
            /*public: inline friend bool operator > (const PdesTime &t1, const PdesTime &t2)
			{
                if(t1._time > t2._time)
					return true;
                else if(t1._time == t2._time && t1._lc > t2._lc)
					return true;
                else if(t1._time == t2._time && t1._lc == t2._lc && t1._id > t2._id)
                {
                    std::cout << "hello2 lc " << t1._lc << " id1 " << t1._id << " id2 " << t2._id << std::endl;
					return true;
                }
				else
					return false;
			}*/
            public: inline friend bool operator == (const PdesTime &t1, const PdesTime &t2)
			{
                //if(t1._time == t2._time && t1._lc == t2._lc && t1._id == t2._id)
				//	return true;
				if(t1._time == t2._time && t1._lc == t2._lc)
                    return true;
				else
					return false;
                //std::cout << "hello3 lc " << t1._lc << " id1 " << t1._id << " id2 " << t2._id << std::endl;
			}
            public: inline friend bool operator != (const PdesTime &t1, const PdesTime &t2)
			{
                return !(t1 == t2);
			}
			//----------------------------------------------------
            public: inline friend ::std::ostream &operator<<(::std::ostream &stream, const PdesTime &t)
            {
                stream << "(time,lc,id)=(" << t._time << "," << t._lc << /*"," << t._id <<*/ ")";
                return stream;
            }
            /*--------------------------------------------*/
        };
        //----------------------------------------------------
	}
};
//**************************************************************************************************************//

#endif

