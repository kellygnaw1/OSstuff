#ifndef __RWLOCK_H__
#define __RWLOCK_H__
#include<semaphore.h>
#include <pthread.h>

class RWLock{
private:
    //Your solution to add more private fields
    int ActiveReaders, WaitingReaders, ActiveWriters, WaitingWriters;
    pthread_cond_t okToRead, okToWrite = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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
