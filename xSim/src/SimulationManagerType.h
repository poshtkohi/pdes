/**
	#define meta ...
	printf("%s\n", meta);
**/

/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#ifndef __Parvicursor_xSim_SimulationManagerType_h__
#define __Parvicursor_xSim_SimulationManagerType_h__

//**************************************************************************************************************//
namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------

		enum SimulationManagerType
		{
			Sequential,
			DeadlockAvoidance,
			DeadlockDetection, 
			TimeWarp
		};
		//----------------------------------------------------
	}
};

//**************************************************************************************************************//

#endif
