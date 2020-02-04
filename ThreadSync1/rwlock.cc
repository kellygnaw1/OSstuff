#include<stdio.h>
#include <pthread.h>
#include <iostream> 


#include "rwlock.h"

//Your solution to implement each of the following methods
//
RWLock::RWLock() {
    ActiveReaders = 0;
    ActiveWriters = 0;
    WaitingReaders = 0;
    WaitingWriters = 0;
}
RWLock::~RWLock() {
    pthread_cond_destroy(&okToWrite);
    pthread_cond_destroy(&okToRead);
    pthread_mutex_destroy(&lock);
}
void RWLock::startRead() {
    pthread_mutex_lock(&lock);

    while (ActiveReaders + ActiveWriters > 0){
        WaitingReaders++;
        pthread_cond_wait(&okToRead,&lock);
        WaitingWriters--;
    }
    ActiveReaders ++;
    pthread_mutex_unlock(&lock);
}

void RWLock::doneRead() {
    pthread_mutex_lock(&lock);
    ActiveReaders --;
    if (ActiveReaders == 0 && WaitingWriters > 0){
        pthread_cond_signal(&okToWrite);
    }
    pthread_mutex_unlock(&lock);

}
void RWLock::startWrite() {
    pthread_mutex_lock(&lock);
    while((ActiveWriters + ActiveReaders) > 0){
        WaitingWriters++;
        pthread_cond_wait(&okToWrite, &lock);
        WaitingWriters--;
    }
    ActiveWriters++;
    pthread_mutex_unlock(&lock);
}
void RWLock::doneWrite() {
    pthread_mutex_lock(&lock);
    ActiveWriters--;
    if(WaitingWriters > 0){
        pthread_cond_signal(&okToWrite);
    }
    else if(WaitingReaders > 0){
        pthread_cond_broadcast(&okToWrite);
    }
    pthread_mutex_unlock(&lock);
}
