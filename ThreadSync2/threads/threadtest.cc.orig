// threadtest.cc
//        Simple test case for the threads assignment.
//
//        Create two threads, and have them context switch
//        back and forth between themselves by calling Thread::Yield,
//        to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.


#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "hashchain.h"
#include <iostream>
using namespace std;
// testnum is set in main.cc
int testnum = 0;
HashMap m;

#if defined(CHANGED) && defined(THREADS)

#define NUMKEYS 100
/* Test operations on hash table */

/* Each thread will use this function to access the hashtable. You can use
 * this to test your locking mechanism used to protect the hash table */
void tfunc(int arg){




}

void ThreadTest1(int childThreadNum)
{
    DEBUG('t', "Entering ThreadTest1");
    // Prepare a new thread to be forked
    Thread *t = new Thread("forked thread");
    // 'Fork' the new thread, putting it on the ready queue, about to run tfunc
    t->Fork(tfunc, childThreadNum);
}


void ThreadTest(int numThreads)
{
    if(numThreads < 0)
    {
        printf("Num threads must be >= 0");
        return;
    }
    // No data races for writing workingThreads because only the parent is running at this time

    // Spawn N child threads
    for(int i = 0; i < numThreads; i++)
    {
        ThreadTest1(i + 1); // Pass the child thread's number (i+1)
    }

}
#endif
