#ifndef __RWLOCK_H__
#define __RWLOCK_H__
//#include<stdio.h>
//#include<pthread.h>
#include<semaphore.h>
#include "synch.h"

class RWLock{
private:
  int w_wait; //how many writers are waiting to write
  int r_wait; //how many readers are waiting to read
  int w_active; //how many writers are actively writing
  int r_active; //how many readers are actively reading
  Lock *lock;
  Condition *Rcond;
  Condition *Wcond;

 
public:
    	RWLock();
    	~RWLock();
    //Reader
    	void startRead();
    	void doneRead();
    // Writer
    	void startWrite();
    	void  doneWrite();
};

#endif
