// processmanager.cc
//

#include "processmanager.h"

//----------------------------------------------------------------------
// ProcessManager::ProcessManager
//  Create a new process manager to manager MAX_PROCESSES processes.
//----------------------------------------------------------------------

ProcessManager::ProcessManager()
{
    pids = new BitMap(MAX_PROCESSES);
    lock = new Lock("ProcessManagerLock");
    PCB_list = new PCB*[MAX_PROCESSES];
    exitLocks = new Lock*[MAX_PROCESSES];
    exitSignals = new Condition*[MAX_PROCESSES];

    for(int i = 0; i < MAX_PROCESSES; i++){
        PCB_list[i] = NULL;
        exitLocks[i] = new Lock("Exit Lock");
        exitSignals[i] = new Condition("Exit Signal");
    }
}

//----------------------------------------------------------------------
// ProcessManager::~ProcessManager
//  Deallocate a process manager.
//----------------------------------------------------------------------

ProcessManager::~ProcessManager()
{
    delete pids;
    for(int i = 0; i < MAX_PROCESSES; i++){
        if(PCB_list[i] != NULL)
            delete PCB_list[i];
        delete exitLocks[i];
        delete exitSignals[i];
    }
    delete[] PCB_list;
    delete[] exitLocks;
    delete[] exitSignals;
    delete lock;
}

//----------------------------------------------------------------------
// ProcessManager::allocPid
//  Allocate an unused PID to be used by a process.
//
//  For now do nothing.
//----------------------------------------------------------------------

int ProcessManager::allocPid()
{
	int pid = pids -> Find();
	return pid;
}

//----------------------------------------------------------------------
// ProcessManager::trackPcb
//  Kepp track of pcb
//
//----------------------------------------------------------------------

void ProcessManager::trackPCB(int pid, PCB *pcb)
{
	this->PCB_list[pid] = pcb;
}

//----------------------------------------------------------------------
// ProcessManager::freePid
//  Deallocate a PID that is in use so that it can be allocated again by
//  another process.
// 	Deallocate PCB spot
//----------------------------------------------------------------------

void ProcessManager::freePid(int pid)
{
	pids -> Clear(pid);
    if(PCB_list[pid] != NULL)
    	delete PCB_list[pid];
	PCB_list[pid] = NULL; 
}

//----------------------------------------------------------------------
// ProcessManager::getPCB
//  Do what its name suggests it do. Give back pointer to PCB
//
//----------------------------------------------------------------------

PCB* ProcessManager::getPCB(int pid)
{
	return PCB_list[pid]; 
}

//----------------------------------------------------------------------
// ProcessManager::waitFor
//  Wait on Condition Variable
//
//----------------------------------------------------------------------

int ProcessManager::waitFor(int childPID, int parentPID)
{
    int exitStatus;
    exitLocks[childPID] -> Acquire();
    fprintf(stderr, "Process %d starts waiting for process %d\n", parentPID, childPID);
    while(PCB_list[childPID]->getExitStatus() == NOT_FINISHED){
        exitSignals[childPID] -> Wait(exitLocks[childPID]);
    }
    exitStatus = PCB_list[childPID]->getExitStatus();
    fprintf(stderr, "Process %d finishes waiting for process %d\n", parentPID, childPID);
    // Deallocate memory if currentThread is the parent Thread
    if( parentPID == PCB_list[childPID] -> parentPid )
        freePid(childPID);
    exitLocks[childPID] -> Release();
    return exitStatus;
}

//----------------------------------------------------------------------
// ProcessManager::broadcastExit
//  Let other process know that I am exiting
//----------------------------------------------------------------------

void ProcessManager::broadcastExit(int pid)
{
    exitLocks[pid] -> Acquire();
    exitSignals[pid] -> Broadcast(exitLocks[pid]);
    exitLocks[pid] -> Release();
}

//----------------------------------------------------------------------
// ProcessManager::waitForChildrenToFinish
//  Wait for All Children to finish
//----------------------------------------------------------------------

void ProcessManager::waitForChildrenToFinish(int parentPID)
{
    for(int i = 0; i < MAX_PROCESSES; i++){
        if(PCB_list[i] != NULL && PCB_list[i] -> parentPid == parentPID){
            waitFor(i, parentPID);
        }
    }
}