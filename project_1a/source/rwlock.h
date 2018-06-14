#ifndef __RWLOCK_H__
#define __RWLOCK_H__
//#include<stdio.h>
//#include<pthread.h>
#include<semaphore.h>

class RWLock{
private:
    pthread_cond_t r_cond; //readers wait on r_cond
    pthread_cond_t w_cond; //writers wait on w_cond
    int w_wait; //how many writers are waiting to write
    int r_wait; //how many readers are waiting to read
    int w_active; //how many writers are actively writing
    int r_active; //how many readers are actively reading
    pthread_mutex_t lock; 

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
