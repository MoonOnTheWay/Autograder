/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 * Use fine-grain rwlock or lock
 **/



#include <iostream>
#include <unistd.h>
//#include "rwlock.h"
#include "hashchain.h"
#include "synch.h"
#define SLEEPLEN 0

#ifdef NACHOS
//For NACHOS
#include "system.h" //for currentThread->Yield()
#define YIELD() currentThread->Yield()
#else
//For pthreads
#define YIELD() pthread_yield()
#endif

#ifdef NOLOCK
//no synchronization code
#define START_READ() do{}while(0)
#define END_READ() do{}while(0)
#define START_WRITE() do{}while(0)
#define END_WRITE() do{}while(0)
#elif defined P1_SEMAPHORE //using nachos semaphore
#define START_READ() sem[hash]->P()
#define END_READ() sem[hash]->V()
#define START_WRITE() sem[hash]->P()
#define END_WRITE() sem[hash]->V()
#elif defined P1_LOCK //using our implemented nachos lock
#define START_READ() lck[hash]->Acquire()
#define END_READ() lck[hash]->Release()
#define START_WRITE() lck[hash]->Acquire()
#define END_WRITE() lck[hash]->Release()
#elif defined P1_RWLOCK //using our rwlock
#define START_READ() rwlck[hash]->startRead()
#define END_READ() rwlck[hash]->doneRead()
#define START_WRITE() rwlck[hash]->startWrite()
#define END_WRITE() rwlck[hash]->doneWrite()
#else //else behave like NOLOCK
#define START_READ() do{}while(0)
#define END_READ() do{}while(0)
#define START_WRITE() do{}while(0)
#define END_WRITE() do{}while(0)
#endif

LinkedHashEntry:: LinkedHashEntry(int key1, int value1) {
  this->key = key1;
  this->value = value1;
  this->next = NULL;
}

int 
LinkedHashEntry:: getKey() {
  return key;
}
int 
LinkedHashEntry:: getValue() {
  return value;
}

void 
LinkedHashEntry:: setValue(int value1) {
  this->value = value1;
}


LinkedHashEntry * 
LinkedHashEntry:: getNext() {
  return next;
}

void 
LinkedHashEntry:: setNext(LinkedHashEntry *next1) {
  this->next = next1;
}

const int TABLE_SIZE = 128;

HashMap::HashMap() {
  table = new LinkedHashEntry*[TABLE_SIZE];
  for (int i = 0; i < TABLE_SIZE; i++)
    table[i] = NULL;
#ifdef P1_SEMAPHORE
  Semaphore * csem;
  for (int i = 0; i < TABLE_SIZE; i++)
  {
    csem = new Semaphore("Fine Grain",1);
    sem[i] = csem;
  }

#elif defined P1_LOCK
  Lock * plk;
  for (int i = 0; i < TABLE_SIZE; i++)
  {
    plk = new Lock("Lock");
    lck[i] = plk;
  }
#elif defined P1_RWLOCK
  RWLock *rw1;

  for (int i = 0; i < TABLE_SIZE; i++)
  {
    rw1 = new RWLock();
    rwlck[i] = rw1;
  }
#endif
}

int 
HashMap::_get(int key) { //internal get() function. DO NOT MODIFY
  int hash = (key % TABLE_SIZE);
  //usleep(SLEEPLEN);
  if (table[hash] == NULL) {
    YIELD();
    return -1;
  } else {
    YIELD();
    LinkedHashEntry *entry = table[hash];
    while (entry != NULL && entry->getKey() != key) {
      entry = entry->getNext();
      YIELD();
    }
    if (entry == NULL) {
      YIELD();
      return -1;
    } else { 
      YIELD();
      return entry->getValue();
    }
  }
  return -1; //should never get here (just for complaining compilers)
}

void
HashMap::_put(int key, int value) { //internal put() function. DO NOT MODIFY
  int hash = (key % TABLE_SIZE);
  if (table[hash] == NULL) {
    YIELD();
    table[hash] = new LinkedHashEntry(key, value);
  } else {
    YIELD();
    LinkedHashEntry *entry = table[hash];
    while (entry->getNext() != NULL && entry->getKey() != key) {
      YIELD();
      entry = entry->getNext();
    }
    if (entry->getKey() == key) {
      YIELD();
      entry->setValue(value);
    } else {
      YIELD();
      entry->setNext(new LinkedHashEntry(key, value));
    }
  }
  YIELD();
}

int 
HashMap::get(int key1) {
  int hash = (key1 % TABLE_SIZE);
  START_READ();
  int value=_get(key1);
  END_READ();
  return value;
}

void 
HashMap::put(int key1, int value1) {
  int hash = (key1 % TABLE_SIZE);
  START_WRITE();
  _put(key1,value1);
  END_WRITE();
}


void
HashMap::remove(int key) { //TODO: make this function threadsafe
  int hash = (key % TABLE_SIZE);
  START_WRITE();
  if (table[hash] != NULL) {
    YIELD();
    LinkedHashEntry *prevEntry = NULL;
    LinkedHashEntry *entry = table[hash];
    while (entry->getNext() != NULL && entry->getKey() != key) {
      YIELD();
      prevEntry = entry;
      entry = entry->getNext();
    }
    if (entry->getKey() == key) {
      YIELD();
      if (prevEntry == NULL) {
        LinkedHashEntry *nextEntry = entry->getNext();
        entry->setNext(NULL);
        delete entry;
        YIELD();
        table[hash] = nextEntry;
      } else {
        LinkedHashEntry *next = entry->getNext();
        entry->setNext(NULL);
        delete entry;
        YIELD();
        prevEntry->setNext(next);
      }
    }
  }
  END_WRITE();
}

HashMap:: ~HashMap() {
  for (int hash = 0; hash < TABLE_SIZE; hash++){
    START_WRITE();
    if (table[hash] != NULL) {
      LinkedHashEntry *prevEntry = NULL;
      LinkedHashEntry *entry = table[hash];
      while (entry != NULL) {
        prevEntry = entry;
        entry = entry->getNext();
        delete prevEntry;
      }
    }
    END_WRITE();
  }
  delete[] table;
}


void
HashMap::increment(int key, int value) { //TODO: make this function threadsafe
  int hash = (key % TABLE_SIZE);
  START_WRITE();
  _put(key,_get(key)+value);
  END_WRITE();
}
