/**
    #define meta ...
    printf("%s\n", meta);
**/


/*
	All rights reserved to Alireza Poshtkohi (c) 1999-2023.
	Email: arp@poshtkohi.info
	Website: http://www.poshtkohi.info
*/

#include "LogicalProcess.h"

UInt32 processCounter = 0;

// Stores the allocated logical processes.
// We use an array to store the references to increase the access performance.
Vector<LogicalProcess *> *logicalProcesses = new Vector<LogicalProcess *>;

Mutex *mtx = new Mutex();

//**************************************************************************************************************//

namespace Parvicursor
{
	namespace xSim
	{
		//----------------------------------------------------
		LogicalProcess::LogicalProcess()
		{
			e = null;
			marked = false;
			visited = false;
			isClockLogicalProcess = false;
			localTime = 0;
			clusterID = 0;
			processId = processCounter;
			processCounter++;
			logicalProcesses->PushBack(this);
            processSpaceStateId = -1;
            lc_time = 0;
			//cout << "LogicalProcess ID " << processId << endl;
			//cout << "LogicalProcess() count : " << Parvicursor::xSim::logicalProcesses.size() << endl;
			//disposed = false;
			initialized = false;
/*#if defined __Delta_Enabled__
			deltaCounter = 0;
#endif*/
			twi = null;
		}
		//----------------------------------------------------
		LogicalProcess::~LogicalProcess()
		{
			//if(!disposed)
			{
				clusterID = 0;
				(*logicalProcesses)[processId] = null;
				// twi is allocated in the TimeWarpSimulationManager class.
				if(twi != null)
					delete twi;
				//disposed = true;
				//cout << "~LogicalProcess()" << endl;
			}
		}
		//----------------------------------------------------
		void LogicalProcess::AddOutputLP(Int32 id)
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			if(id < 0)
				throw ArgumentOutOfRangeException("id", "id is less than zero.");

			/*for(register Int32 i = 0 ; i < outputLPs.size() ; i++)
				if(outputLPs[i] == id)
					throw ArgumentException("id", "The logical processor already exists");*/

			outputLPs.push_back(id);
			outputLPsMap[id] = id;
		}
		//----------------------------------------------------
		void LogicalProcess::AddInputLP(Int32 id)
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			if(id < 0)
				throw ArgumentOutOfRangeException("id", "id is less than zero");

			/*for(register Int32 i = 0 ; i < inputLPs.size() ; i++)
				if(inputLPs[i] == id)
					throw ArgumentException("id", "The logical processor already exists.");*/

			inputLPs.push_back(id);
		}
		//----------------------------------------------------
		/*void LogicalProcess::SendEvent(EventInterface *e)
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			//if(e == null)
			//	throw ArgumentNullException("e", "e is null");

			manager->SendEvent(e);
		}
		//----------------------------------------------------
		EventInterface *LogicalProcess::GetNextEvent()
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			return manager->GetNextEvent(this);
		}
		//----------------------------------------------------
		EventInterface *LogicalProcess::PeekNextEvent()
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			return manager->PeekNextEvent(this);
		}
		//----------------------------------------------------
		UInt64 LogicalProcess::GetNextEventTime()
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			return manager->GetNextEventTime(this);
		}*/
		//----------------------------------------------------
		/*UInt64 LogicalProcess::GetID()
		{
			if(disposed)
				throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			return processId;
		}*/
		//----------------------------------------------------
		Int32 LogicalProcess::GetOutputLpCount()
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			return (Int32)outputLPs.size();
		}
		//----------------------------------------------------
		Int32 LogicalProcess::GetInputLpCount()
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			return (Int32)inputLPs.size();
		}
		//----------------------------------------------------
		Int32 LogicalProcess::GetOutputLpId(Int32 index)
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			if(index < 0 || index > (Int32)outputLPs.size() - 1)
				throw ArgumentOutOfRangeException("index", "index must be greater than or equal to zero and be less than GetOutputLpCount()");

			return outputLPs[index];
		}
		//----------------------------------------------------
		Int32 LogicalProcess::GetInputLpId(Int32 index)
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			if(index < 0 || index > (Int32)inputLPs.size() - 1)
				throw ArgumentOutOfRangeException("index", "index must be greater than or equal to zero and be less than GetInputLpCount()");

			return inputLPs[index];
		}
		//----------------------------------------------------
		/*LogicalProcess *LogicalProcess::GetLogicalProcessById(UInt64 id)
		{
			//if(id < 0)
			//	throw ArgumentOutOfRangeException("id", "id is less than zero");

			return manager->GetLogicalProcessById(id);
		}*/
		//----------------------------------------------------
		/*UInt64 LogicalProcess::GetSimulationTime()
		{
			if(disposed)
				throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			return localTime;
		}*/
		//----------------------------------------------------
		/*void LogicalProcess::SetSimulationTime(UInt64 localTime)
		{
			if(disposed)
				throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			if(localTime < 0)
				throw ArgumentOutOfRangeException("localTime", "localTime is less than zero");

			this->localTime = localTime;
		}*/
		//----------------------------------------------------
		void LogicalProcess::SetSimulationManager(SimulationManagerInterface *manager)
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			if(manager == null)
				throw ArgumentNullException("manager", "manager is null");

			this->manager = manager;
		}
		//----------------------------------------------------
		/*bool LogicalProcess::HaveMoreEvents()
        {
            // does this guy have any more events to process at this time ?
            EventInterface *event =  manager->PeekNextEvent(this);

            if(event == null)
                return false;
            else
            {
                // return true only if the next event is for the same destination
                // and is at the current simulation time.
                if(event->GetReceiveTime() == GetSimulationTime())
                {
                    if(event->GetReceiverReference() == GetID())
                    {
                        //std::cout << "HaveMoreEvents()" << std::endl;
                        return true;
                    }
                    else
                        return false;
                }
                else
                    return false;
            }
        }*/
		//----------------------------------------------------
		/*Int32 LogicalProcess::GetOwnerClusterID()
		{
			if(disposed)
				throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			return clusterID;
		}*/
		//----------------------------------------------------
		void LogicalProcess::SetOwnerClusterID(UInt32 id)
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");

			//if(id < 0)
			//	throw ArgumentOutOfRangeException("id", "id is less than zero");

			clusterID = id;
		}
        //----------------------------------------------------
		void LogicalProcess::AxillaryMethod(Object *state)
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");
		}
        //----------------------------------------------------
		void LogicalProcess::Elaborate()
		{
			//if(disposed)
			//	throw ObjectDisposedException("LogicalProcess", "The LogicalProcess has been disposed");
		}
		//----------------------------------------------------
		void LogicalProcess::Terminate()
		{
			if(manager->GetSimulationManagerType() == DeadlockAvoidance || manager->GetSimulationManagerType() == DeadlockDetection)
                throw InvalidOperationException("Terimination is not currently supported for DeadlockAvoidance/DeadlockDetection protocol");

            if(manager->GetSimulationManagerType() == Sequential)
            {
                LogicalProcess *receiver = (*logicalProcesses)[0];
                EventInterface *newEvent = new EventInterface();
                newEvent->SetEventType(TerminationHasRequestedMessageByUser);
                newEvent->SetSenderReference(GetID());
                newEvent->SetReceiverReference(receiver->GetID());
                newEvent->SetSendTime(GetSimulationTime());
                newEvent->SetReceiveTime(GetSimulationTime());
                //if(GetLogicalProcessById(receiver)->GetOwnerClusterID() != GetOwnerClusterID())
                //	manager->IncrementTerminationCounter(GetOwnerClusterID());
                SendEvent(newEvent);
            }
			/*else if(manager->GetSimulationManagerType() == DeadlockDetection)
			{
				///std::cout << "LogicalProcess::Terminate() GetSimulationTime() " << GetSimulationTime() << " by " << GetName().get_BaseStream() << endl;

				for(register Int32 i = 0 ; i < GetOutputLpCount() ; i++)
				{
					UInt32 receiver = GetOutputLpId(i);
					EventInterface *newEvent = new EventInterface();
					newEvent->SetEventType(TerminationHasRequestedMessageByUser);
					newEvent->SetSenderReference(GetID());
					newEvent->SetReceiverReference(receiver);
					newEvent->SetSendTime(GetSimulationTime());
					newEvent->SetReceiveTime(GetSimulationTime());
					//if(GetLogicalProcessById(receiver)->GetOwnerClusterID() != GetOwnerClusterID())
					//	manager->IncrementTerminationCounter(GetOwnerClusterID());
					SendEvent(newEvent);
					//break;
				}

				//manager->SetRequestToTerminateExecution(GetOwnerClusterID());
			}*/
		}
		//----------------------------------------------------
	};
};
//**************************************************************************************************************//
