// pcb.cc
//

#include "pcb.h"
#include <cstddef>

//----------------------------------------------------------------------
// PCB::PCB
//  Allocate a new process control block and initialize it with process
//  ID and a parent.
//----------------------------------------------------------------------

PCB::PCB(int pid, int parentPid)
{
    this->pid = pid;
    this->parentPid = parentPid;
    this->exitStatus = NOT_FINISHED;
    this-> openFiles = new UserOpenFile*[MAX_OPEN_FILES];
    for(int i = 0; i < MAX_OPEN_FILES; i++){
    	openFiles[i] = NULL;
    }
}

//----------------------------------------------------------------------
// PCB::~PCB
//  Deallocate a process control block.
//----------------------------------------------------------------------

PCB::~PCB()
{
	for(int i = 0; i < MAX_OPEN_FILES; i++){
 		if(openFiles[i] != NULL){
 			delete openFiles[i];
 		}   	
    }
    delete[] openFiles;
}

void PCB::setExitStatus(int status){ this-> exitStatus = status;}

int PCB::getExitStatus(){ return exitStatus;}

int PCB::addOpenFile(UserOpenFile* openFile)
{
  for(int i = 3; i < MAX_OPEN_FILES; i++){
    if(openFiles[i] == NULL){
      openFiles[i] = openFile;
      return i;
    }   	
  }
  return -1;
}

void PCB::closeOpenFile(int fd)
{
	if(openFiles[fd] != NULL){
		delete openFiles[fd];
	}
	openFiles[fd] = NULL;
}


PCB* PCB::ForkHelper(int pid, int parentPid){
	PCB* ChildPCB = new PCB(pid, parentPid);
	for(int i = 0; i < MAX_OPEN_FILES; i++){
		if(this -> openFiles[i] == NULL){
			ChildPCB -> openFiles[i] = NULL;
		}
		else{
			ChildPCB -> openFiles[i] = new UserOpenFile(this -> openFiles[i]);
		}
	}
	return ChildPCB;
}
