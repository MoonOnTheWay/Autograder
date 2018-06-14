// exception.cc
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include <stdio.h>
#include <string.h>
#include "copyright.h"
#include "syscall.h"
#include "system.h"
#include "machine.h"
#include "pcb.h"
#include "../threads/system.h"
#include "../machine/machine.h"
#include "addrspace.h"
#include "../filesys/openfile.h"

#define MAX_FILENAME 256
#define INITIAL_FILE_SIZE 1024 //This can be anything since the underlying structure is Linux

SpaceId doFork();
void ForkBridge(int newProcessPC);
void doExit();
int doExec(char* filename);
void doYield();
int doJoin();
// Part 2
void doCreate(char* filename);
int doOpen(char* filename);
void doWrite();
int doRead();
void doClose();
// Helper Functions to help moving memory from user space to kernel space
void readFilenameFromUsertoKernel(char* filename);
int moveBytesFromMemToKernel(int virtAddr, char* buffer, int size);
int userRead(int virtAddr, char* buffer, int size);


//----------------------------------------------------------------------
// ExceptionHandler
//  Entry point into the Nachos kernel.  Called when a user program
//  is executing, and either does a syscall, or generates an addressing
//  or arithmetic exception.
//
//  For system calls, the following is the calling convention:
//
//  system call code -- r2
//    arg1 -- r4
//    arg2 -- r5
//    arg3 -- r6
//    arg4 -- r7
//
//  The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//  "which" is the kind of exception.  The list of possible exceptions
//  are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int result, type = machine->ReadRegister(2);
    char* filename = new char[MAX_FILENAME];

    if (which == SyscallException) {
        switch (type) {
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            case SC_Fork:
                DEBUG('a', "Fork() system call invoked.\n");
                result = doFork();
                machine->WriteRegister(2, result);
                break;
            case SC_Exit:
                DEBUG('a', "Exit() system call invoked.\n");
                doExit();
                break;
            case SC_Exec:
                DEBUG('a', "Exec() system call invoked.\n");
                readFilenameFromUsertoKernel(filename);
                result = doExec(filename);
                machine->WriteRegister(2, result);
                break;
            case SC_Yield:
                DEBUG('a', "Yield() system call invoked.\n");
                doYield();
                break;
            case SC_Join:
                DEBUG('a', "Join() system call invoked.\n");
                result = doJoin();
                machine->WriteRegister(2, result);
                break;
            //PART 2
            case SC_Create:
                DEBUG('a', "Create() system call invoked.\n");
                readFilenameFromUsertoKernel(filename);
                doCreate(filename);
                break;
            case SC_Open:
                DEBUG('a', "Open() system call invoked.\n");
                readFilenameFromUsertoKernel(filename);
                result = doOpen(filename);
                machine->WriteRegister(2, result);
                break;
            case SC_Read:
                DEBUG('a', "Read() system call invoked.\n");
                result = doRead();
                machine->WriteRegister(2, result);
                break;
            case SC_Write:
	      DEBUG('a', "Write() system call invoked.\n");
                doWrite();
                break;
            case SC_Close:
                DEBUG('a', "Close() system call invoked.\n");
                doClose();
                break;
            default:
                printf("Unexpected system call %d. Halting.\n", type);
                interrupt->Halt();
        }
    } else {
        printf("Unexpected user mode exception %d. Halting.\n", which);
        interrupt->Halt();
    }

    // Increment program counter
    machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4);
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PrevPCReg) + 4);

}

//----------------------------------------------------------------------
// doExit
//----------------------------------------------------------------------

void doExit()
{
    //Get status code
    int status = machine->ReadRegister(4);
    //Set the exit status in the PCB of this process 
    int curPID = currentThread->space->getpid();
    PCB* curPCB = processManager -> getPCB(curPID);
    curPCB->setExitStatus(status);

    //Clean up the space of this process
    //fprintf(stderr, "Freeing addr space of  %d\n", currentThread -> space -> getpid());    
    delete currentThread->space;
    //fprintf(stderr, "Done Freeing");
    currentThread->space = NULL;

    //Wait for Children to Finish to Deallocate all the resources
    processManager->waitForChildrenToFinish(curPID);
    
    //If got to here, then we can know all children have finished, now we can finish or Halt

    //Let other processes know this process  exits.
    processManager -> broadcastExit(curPID);

    fprintf(stderr, "Process %d exiting\n", curPID);
    if(curPID == 0)
        interrupt->Halt();
    currentThread->Finish();
}

//----------------------------------------------------------------------
// execLauncher
//----------------------------------------------------------------------

void execLauncher(int unused)
{
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
    machine->Run();
}

//----------------------------------------------------------------------
// doExec
//----------------------------------------------------------------------

int doExec(char* filename)
{
    PCB *childPcb;
    int parentPid, childPid;
    OpenFile *execFile;
    AddrSpace* childSpace;
    Thread* childThread;

    // Finally it needs an address space. We will initialize the address
    // space by loading in the program found in the executable file that was
    // passed in as the first argument.
    execFile = fileSystem->Open(filename);
    if (execFile == NULL) {
        fprintf(stderr, "Unable to open file %s for execution. Terminating.\n", filename);
        return -1;
    }

    childSpace = new AddrSpace(execFile);
    if(!childSpace->isValid()){
        delete childSpace;
        delete execFile;
        return -1;
    }

    // Initialize new PCB
    childPid = childSpace -> getpid();
    parentPid = currentThread -> space -> getpid();
    childPcb = new PCB(childPid, parentPid);

    processManager -> trackPCB(childPid, childPcb);

    // The new process needs a kernel thread by which we can manage its state
    childThread = new Thread(filename);

    // Link everything together
    childThread -> space = childSpace;
    childPcb -> thread = childThread;
    
    delete execFile;
    
    // We launch the process with the kernel threads Fork() function. Note
    // that this is different from our implementation of Fork()!
    childPcb->thread->Fork(execLauncher, 0);
    fprintf(stderr, "Process %d exec-ing process %d\n", parentPid, childPid);
    // Because we just overwrote the current thread's address space when we
    // called `new AddrSpace(execFile)` it can no longer be allowed to
    // execute so we call Finish(). You will have to fix this in your
    // implementation once you implement multiple address spaces.
    doYield();
    return childPid;
}

SpaceId doFork()
{
    // Sanity Check
    if (currentThread->space->getNumPages() > memoryManager->getNumFreeFrames() || 
        processManager->getNumPidAvail() <= 0)
        return -1;

    // Func1 address is in register 4
    int Func1Addr = machine->ReadRegister(4);

    // Create a new kernel thread for the child
    Thread* child_Thread = new Thread("child of Fork()");

    // Create a duplicate address space of current process
    AddrSpace *dupAddrSpace = new AddrSpace(currentThread->space);
    
    // Create Child PCB
    int parentPid, childPid;
    parentPid = currentThread->space->getpid();
    childPid = dupAddrSpace->getpid();

    fprintf(stderr, "Process %d just forks process  %d\n", parentPid, childPid);

    PCB *childPcb = processManager->getPCB(parentPid) -> ForkHelper(childPid, parentPid);
    processManager-> trackPCB(childPid, childPcb);
    childPcb -> thread = child_Thread;
    childPcb -> thread -> space = dupAddrSpace;
    // New thread runs a dummy function creates a bridge for execution of the user function
    childPcb->thread->Fork(ForkBridge, Func1Addr);
    // Current thread Yield so new thread can run
    doYield();
    return childPid;
}

void ForkBridge(int newProcessPC)
{
    // Get fresh registers, but use copy of addr space
    currentThread->space->InitRegisters();
    // currentThread->RestoreUserState();
    currentThread->space->RestoreState();

    // Set the PC and run
    machine->WriteRegister(PCReg, newProcessPC);
    machine->WriteRegister(PrevPCReg, newProcessPC - 4);
    machine->WriteRegister(NextPCReg, newProcessPC + 4);

    machine->Run();
    ASSERT(FALSE); // should never reach here
}

void doYield()
{
  currentThread->SaveUserState();
  //This kernel thread yields
  currentThread->Yield();
  //Now this process is resumed for execution after yielding.
  //Restore the corresponding user process's states (both registers and page table)
  //Save the corresponding user process's register states.
  currentThread->RestoreUserState();
}


int doJoin()
{
    // Read the child Process Id
    int childPID = machine->ReadRegister(4);
    // Wait for child Process to Finish and get exitStatus
    int childExitStatus = processManager->waitFor(childPID, currentThread->space->getpid());
    // Return exit status
    return childExitStatus;
}

/*
----------------------------PART 2: File Sys Call-----------------------
*/

//----------------------------------------------------------------------
// doCreate
//----------------------------------------------------------------------

void doCreate(char* filename)
{
    bool success = fileSystem->Create(filename, INITIAL_FILE_SIZE);
    // if(success)
    //     fprintf(stderr, "Process %d creates %s\n", currentThread -> space -> getpid(), filename);
    // else
    //     fprintf(stderr, "Process %d fails to create %s\n", currentThread -> space -> getpid(), filename);
}

//----------------------------------------------------------------------
// doOpen
//----------------------------------------------------------------------

int doOpen(char* filename)
{
    UserOpenFile* userOpenFile;
    int fd;

    OpenFile* file = fileSystem->Open(filename);
    if(file == NULL){
        fprintf(stderr, "%s not found\n", filename);
        return -1;
    }
    
    int fileTableIndex = openFileManager -> addOpenFile(file, filename);
    if(fileTableIndex == FAILED_TO_ADD) return -1;
    userOpenFile = new UserOpenFile(filename, fileTableIndex, 0);
    fd = processManager->getPCB(currentThread->space->getpid())->addOpenFile(userOpenFile);
    return fd;
}


//----------------------------------------------------------------------
// doRead
//----------------------------------------------------------------------

int doRead()
{
  int userBufVirtAddr = machine->ReadRegister(4);
  int userBufSize = machine->ReadRegister(5);
  int dstFile = machine->ReadRegister(6);

  int bytesRead = 0;
  char* buf = new char[userBufSize + 1];
  int actualBytesRead;
  
  if (dstFile == ConsoleInput) {
    //use UserConsoleGetChar() from system.h to save data in internal buffer
    //read until \n
    while(bytesRead < userBufSize && UserConsoleGetChar() != '\n') {
      buf[bytesRead] = UserConsoleGetChar();
      bytesRead++;
    }
  }
  else {
    //find current read offset
    //use openFile::ReadAt in openFile.h to read data
    int curPID = currentThread->space->getpid();
    UserOpenFile* userFile = processManager->getPCB(curPID)->getOpenFile(dstFile);
    OpenFile* ofile = openFileManager->getOpenFile(userFile->fileTableIndex)->openFile;

    //openFileManager->consoleReadLock[userFile->fileTableIndex]->Acquire();
    actualBytesRead = ofile->ReadAt(buf, userBufSize, userFile->currentPosition);
    userFile->currentPosition += actualBytesRead;
    //openFileManager->consoleReadLock[userFile->fileTableIndex]->Release();
  }

  //copy data from internal buffer to destination memory location
  //copy one byte by one
  actualBytesRead = userRead(userBufVirtAddr, buf, userBufSize);
  delete[] buf;
  return actualBytesRead;
}


//----------------------------------------------------------------------
// doWrite
//----------------------------------------------------------------------

void doWrite()
{
  int userBufVirtAddr = machine->ReadRegister(4);
  int userBufSize = machine->ReadRegister(5);
  int dstFile = machine->ReadRegister(6);
  int actualBytesWritten = 0;
  
  //allocate internal buffer
  char* internalBuf = new char[userBufSize + 1];

  //copy dta from source memory location to internal buffer
  if(dstFile == ConsoleOutput) {
    moveBytesFromMemToKernel(userBufVirtAddr, internalBuf, userBufSize);
    internalBuf[userBufSize] = 0;
    //printf("%s", internalBuf);
    openFileManager->consoleWriteLock->Acquire();
    for(int i = 0; i < userBufSize; i++) {
      UserConsolePutChar(internalBuf[i]);
    }
    openFileManager->consoleWriteLock->Release();
  }
  else {
    //find current write offset
    //use openFile::WriteAt (in openfile.h) to write data to file
    moveBytesFromMemToKernel(userBufVirtAddr, internalBuf, userBufSize);
    int curPID = currentThread->space->getpid();
    UserOpenFile* userFile = processManager->getPCB(curPID)->getOpenFile(dstFile);
    OpenFile* ofile = openFileManager->getOpenFile(userFile->fileTableIndex)->openFile;

    //openFileManager->consoleWriteLock->Acquire();
    actualBytesWritten = ofile->WriteAt(internalBuf,
					userBufSize, userFile->currentPosition);
    userFile->currentPosition += actualBytesWritten;
    //openFileManager->consoleWriteLock->Release();
  }
  
  delete[] internalBuf;
}

//----------------------------------------------------------------------
// doClose
//----------------------------------------------------------------------

void doClose()
{
    int fd = machine->ReadRegister(4);
    PCB* curPCB = processManager->getPCB(currentThread->space->getpid());
    UserOpenFile* userOpenFile = curPCB->getOpenFile(fd);
    if(userOpenFile == NULL){
        fprintf(stderr, "Can't find file with fd %d in Process %d\n", fd, currentThread -> space -> getpid());
        return;
    }
    int fileTableIndex = userOpenFile->fileTableIndex;
    openFileManager -> reduceProccessOpeningOf(fileTableIndex);
    curPCB -> closeOpenFile(fd);
}



/*
---------------------------- Helper Functions -----------------------
*/

//----------------------------------------------------------------------
// Helper function to bring a string that represents a filename from 
// user space to kernel space.
//----------------------------------------------------------------------

void readFilenameFromUsertoKernel(char* filename) {

    int currPosition = 0;
    int filenameArg = machine->ReadRegister(4);

    do {
        moveBytesFromMemToKernel(filenameArg, filename + currPosition, 1);
        filenameArg++;
    } while (filename[currPosition++] != 0);

    filename[currPosition] = 0; // terminate C string
}

//----------------------------------------------------------------------
// Helper function that brings bytes from memory to kernel space.
//----------------------------------------------------------------------

int moveBytesFromMemToKernel(int virtAddr, char* buffer, int size) {

    int physAddr = 0;
    int remainingFromPage = 0;
    int bytesCopied = 0;
    int bytesToCopy = 0;
    
    while (size > 0) {
        physAddr = currentThread->space->Translate(virtAddr);
        remainingFromPage = PageSize - physAddr % PageSize;
        bytesToCopy = remainingFromPage < size ? remainingFromPage : size;
        bcopy(machine->mainMemory + physAddr, buffer + bytesCopied, bytesToCopy);
        size -= bytesToCopy;
        bytesCopied += bytesToCopy;
        virtAddr += bytesToCopy;
    }

    return bytesCopied;
}

int userRead(int virtAddr, char* buffer, int size) {

  int physAddr = 0;
  int numBytesFromPSLeft = 0;
  int numBytesCopied = 0;
  int numBytesToCopy = 0;

  while(size > 0) {
    physAddr = currentThread->space->Translate(virtAddr);
    numBytesFromPSLeft = PageSize - physAddr % PageSize;
    numBytesToCopy = (numBytesFromPSLeft < size) ? numBytesFromPSLeft: size;
    bcopy(buffer + numBytesCopied, machine->mainMemory + physAddr, numBytesToCopy);
    numBytesCopied += numBytesToCopy;
    size -= numBytesToCopy;
    virtAddr += numBytesToCopy;
  }
  return numBytesCopied;
}
