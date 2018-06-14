#include "rwlock.h"
#include "system.h" //for currentThread->Yield()

#ifdef P1_SEMAPHORE
#define LOCK() do{}while(0)
#define UNLOCK() do{}while(0)
#else
#define LOCK() do{}while(0)
#define UNLOCK() do{}while(0)
#define WAIT_R() do{}while(0)
#define WAIT_W() do{}while(0)
#define SIGNAL_W() do{}while(0)
#define BCAST_R() do{}while(0)
#endif

//in case you want to insert yields for testing
#define YIELD() currentThread->Yield()

#ifndef P1_RWLOCK
//non-rwlock mode
#ifdef P1_SEMAPHORE
//semaphore-only code
RWLock::RWLock() {}
void RWLock::startRead() { }
void RWLock::doneRead() { }
void RWLock::startWrite() { }
void RWLock::doneWrite() { }
#else
//lock-only code
RWLock::RWLock() {}
void RWLock::startRead() { }
void RWLock::doneRead() { }
void RWLock::startWrite() { }
void RWLock::doneWrite() { }
#endif
//general non-rwlock code
RWLock::~RWLock() { }
#else 
//rwlock code
RWLock::RWLock() { }
RWLock::~RWLock() { }
void RWLock::startRead() { }
void RWLock::doneRead() { }
void RWLock::startWrite() { }
void RWLock::doneWrite() { }
#endif
