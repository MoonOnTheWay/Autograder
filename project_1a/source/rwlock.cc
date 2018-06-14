#include<stdio.h>
#include <pthread.h>
#include <iostream> //


#include "rwlock.h"

RWLock::RWLock() {
    pthread_mutex_init(&lock,0);
    pthread_cond_init(&r_cond,0);
    pthread_cond_init(&w_cond,0);
    w_wait=0;
    w_active=0;
    //r_wait=0;
    r_active=0;
}
RWLock::~RWLock() {
}
void RWLock::startRead() {
    pthread_mutex_lock(&lock);
    //if a writer is active or waiting, we need to wait (write-priority)
    if (w_active || w_wait) {
      //++r_wait;
      do {
        pthread_cond_wait(&r_cond,&lock);
      } while(w_active || w_wait);
      //--r_wait;
    }
    ++r_active;
    pthread_mutex_unlock(&lock);
}
void RWLock::doneRead() {
    pthread_mutex_lock(&lock);
    if (0 == --r_active) pthread_cond_signal(&w_cond);
    pthread_mutex_unlock(&lock);
}
void RWLock::startWrite() {
    pthread_mutex_lock(&lock);
    if (r_active || w_active) { //if anyone has the lock we need to wait until lock is free
      ++w_wait;
      do {
        pthread_cond_wait(&w_cond,&lock);
      } while(r_active || w_active);
      --w_wait;
    }
    ++w_active;
    pthread_mutex_unlock(&lock);
}
void RWLock::doneWrite() {
    pthread_mutex_lock(&lock);
    --w_active;
    if (w_wait) pthread_cond_signal(&w_cond);
    else pthread_cond_broadcast(&r_cond);
    pthread_mutex_unlock(&lock);
}
