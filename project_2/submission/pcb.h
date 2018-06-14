// pcb.h
//  The process control block is a data structure used to keep track of a
//	single user process. For now, each process has an ID, a parent and a
//	pointer to its kernel thread.

#ifndef PCB_H
#define PCB_H

#include "useropenfile.h"

class Thread;

#define NOT_FINISHED -1
#define NO_PARENT_PID -1
#define MAX_OPEN_FILES 32


class PCB {

public:
    PCB(int pid, int parentPid);
    ~PCB();

    PCB* ForkHelper(int pid, int parentPid);
    void setExitStatus(int status);
    int getExitStatus();
    int addOpenFile(UserOpenFile* openFile);
    void closeOpenFile(int fd);
    UserOpenFile* getOpenFile(int fd){ return openFiles[fd];}


    int exitStatus;
    int pid;            	// Process ID
    int parentPid;      	// Parent's Process ID
    Thread *thread;     	// Kernel thread that controls this process has the registers, 
    						// PC, and address_space
    UserOpenFile** openFiles; // Used to track open files in the Process (FD)
};

#endif // PCB_H
