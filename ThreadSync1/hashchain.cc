/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 *
 **/

//Reference hashchain.cc implementation for proj1b. Note that lots of yields are inserted to help uncover synchronization bugs


#include <iostream>
#include <unistd.h>
#include "hashchain.h"

//NOYIELD disables the yields used to help uncover synchronization bugs
//  - the yields are useful for uncovering bugs (and MUST be left in for the actual submission), but performance testing may be easier without them
//#define NOYIELD 1

#ifndef NOYIELD
#ifdef NACHOS
#include "system.h" //for currentThread->Yield()
#define YIELD() currentThread->Yield()
#else //else pthreads
#define YIELD() pthread_yield()
#endif
#else //else NOYIELD
#define YIELD() do{}while(0)
#endif


#ifdef NOLOCK
//no synchronization code
#define START_READ() do{}while(0)
#define END_READ() do{}while(0)
#define START_WRITE() do{}while(0)
#define END_WRITE() do{}while(0)
#define SYNC_INIT
#define SYNC_DESTROY

#else //else use synchronization

#ifdef P1_RWLOCK //rwlock

#define START_READ() locks[hash].startRead()
#define END_READ() locks[hash].doneRead()
#define START_WRITE() locks[hash].startWrite()
#define END_WRITE() locks[hash].doneWrite()
#define SYNC_INIT locks = new RWLock[TABLE_SIZE]();
#define SYNC_DESTROY delete[] locks;

#else //mutex

#define START_READ() pthread_mutex_lock(&locks[hash])
#define END_READ() pthread_mutex_unlock(&locks[hash])
#define START_WRITE() pthread_mutex_lock(&locks[hash])
#define END_WRITE() pthread_mutex_unlock(&locks[hash])
#define SYNC_INIT \
  locks = new pthread_mutex_t[TABLE_SIZE]; \
  for (int i = 0; i < TABLE_SIZE; i++) { \
    pthread_mutex_init(&locks[i],0); \
  }
#define SYNC_DESTROY delete[] locks;

#endif
#endif


LinkedHashEntry:: LinkedHashEntry(int key, int value) {
  this->k = key;
  this->v = value;
  this->next = NULL;
}

int 
LinkedHashEntry:: getKey() {
  return k;
}
int 
LinkedHashEntry:: getValue() {
  return v;
}

void 
LinkedHashEntry:: setValue(int value) {
  this->v= value;
}


LinkedHashEntry * 
LinkedHashEntry:: getNext() {
  return next;
}

void 
LinkedHashEntry:: setNext(LinkedHashEntry *new_next) {
  this->next = new_next;
}


const int TABLE_SIZE = 128;

HashMap::HashMap() {
  table = new LinkedHashEntry*[TABLE_SIZE];
  for (int i = 0; i < TABLE_SIZE; i++)
    table[i] = NULL;

  SYNC_INIT //execute initialization code for our synchronization
}

int 
HashMap::get(int key) { 
  int hash = (key % TABLE_SIZE);
  START_READ();
  int value=_get(key);
  END_READ();
  return value;
}

void 
HashMap::put(int key, int value) { 
  int hash = (key % TABLE_SIZE);
  START_WRITE();
  _put(key,value);
  END_WRITE();
}


int 
HashMap::_get(int key) { //internal get() function. DO NOT MODIFY
  int hash = (key % TABLE_SIZE);
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


void
HashMap::remove(int key) { 
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

void
HashMap::increment(int key, int value) { 
  int hash = (key % TABLE_SIZE);
  START_WRITE();
  _put(key,_get(key)+value);
  END_WRITE();
}

HashMap::~HashMap() {
  for (int i = 0; i < TABLE_SIZE; i++)
    if (table[i] != NULL) {
      LinkedHashEntry *prevEntry = NULL;
      LinkedHashEntry *entry = table[i];
      while (entry != NULL) {
        prevEntry = entry;
        entry = entry->getNext();
        delete prevEntry;
      }
    }
  delete[] table;
  SYNC_DESTROY //execute destructor code for our synchronization
}


