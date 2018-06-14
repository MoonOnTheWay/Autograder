#include<stdio.h>
#include <iostream> //


#include "rwlock.h"


RWLock:: RWLock(){
  lock  = new Lock("Rwlock");
  Rcond = new Condition("Rcond");
  Wcond = new Condition("Wcond");
  w_wait=0;
  w_active=0;
  //r_wait=0;
  r_active=0;
}

RWLock:: ~RWLock(){
}

void
RWLock::  startRead(){
  lock->Acquire();
  //if a writer is active or waiting, we need to wait (write-priority)
  if (w_active || w_wait) {
    //++r_wait;
    do {
      Rcond->Wait(lock);
    } while(w_active || w_wait);
    //--r_wait;
  }
  ++r_active;
  lock->Release();
}


void
RWLock::  doneRead(){
  lock->Acquire();
  if (0 == --r_active) Wcond->Signal(lock);
  lock->Release();
}

void
RWLock::  startWrite(){
  lock->Acquire();
  if (r_active || w_active) { //if anyone has the lock we need to wait until lock is free
    ++w_wait;
    do {
      Wcond->Wait(lock);
    } while(r_active || w_active);
    --w_wait;
  }
  ++w_active;
  lock->Release();
}

void
RWLock::  doneWrite(){
  lock->Acquire();
  --w_active;
  if (w_wait) Wcond->Signal(lock);
  else Rcond->Broadcast(lock);
  lock->Release();
}


