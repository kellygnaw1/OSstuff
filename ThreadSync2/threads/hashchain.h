/**
 * Code is based on
 *http://www.algolist.net/Data_structures/Hash_table/Chaining
 *
 **/

#include "synch.h"
#include "rwlock.h"
#define HTABLE_SIZE 128
class LinkedHashEntry {
private:
      int key;
      int value;
      LinkedHashEntry *next;
public:
      LinkedHashEntry(int key, int value); 
      int getKey(); 
      int getValue();
      void setValue(int value);
 
      LinkedHashEntry *getNext(); 
      void setNext(LinkedHashEntry *next); 
};


class HashMap {
private:
      LinkedHashEntry **table;
#ifdef P1_SEMAPHORE
      Semaphore  *sem[HTABLE_SIZE];
#elif defined P1_LOCK
      Lock  *lck[HTABLE_SIZE];
#elif  defined P1_RWLOCK
      RWLock *rwlck[HTABLE_SIZE];	
#endif
      void _put(int key, int value);
      int _get(int key);

public:
      HashMap(); 
      int get(int key); 
      void put(int key, int value); 
      void remove(int key); 
      void increment(int key, int value); //increase key by value (or init key to zero)
      ~HashMap(); 
};


