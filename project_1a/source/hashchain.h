/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 *
 **/
#include<semaphore.h>

//the macro NOLOCK disables synchronization
//#define NOLOCK

#ifndef NOLOCK
#include "rwlock.h"
#endif

class LinkedHashEntry {
private:
      int k;
      int v;
      LinkedHashEntry *next;
public:
      LinkedHashEntry(int key, int value); 
      int getKey(); 
      int getValue();
      void setValue(int value);
 
      LinkedHashEntry *getNext(); 
      void setNext(LinkedHashEntry *new_next); 
};


class HashMap {
private:
      LinkedHashEntry **table;
#ifndef NOLOCK
#ifdef P1_RWLOCK //rwlock
#ifdef FINEGRAIN //fine rwlock
      RWLock *locks;
#else //coarse rwlock
      RWLock lock;
#endif
#else //mutex
#ifdef FINEGRAIN //fine mutex
      pthread_mutex_t *locks; 
#else //coarse mutex
      pthread_mutex_t lock; 
#endif
#endif
#endif

      int _get(int key); //internal get function (not threadsafe)
      void _put(int key, int value); //internal put function (not threadsafe)
public:
      HashMap(); 
      int get(int key);  //get value of key (or return -1)
      void put(int key, int value);  //put key,value pair
      void remove(int key); //delete key
      void increment(int key, int value); //increase key by value (or init key to zero)

      ~HashMap(); 
};
