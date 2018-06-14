#include<stdio.h>
#include <pthread.h>
#include <iostream> //


#include "rwlock.h"

RWLock::RWLock() { }
RWLock::~RWLock() { }
void RWLock::startRead() { }
void RWLock::doneRead() { }
void RWLock::startWrite() { }
void RWLock::doneWrite() { }
