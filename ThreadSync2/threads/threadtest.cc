// ptest.cc
//        Set of test cases for Project 1A and 1B
//
//  NOTES:
//    - uses testnum (parsed in main.cc as -q flag) to determine which test(s) to run
//      - -1 means run all tests
//      - 1-n are the individual tests (where there are n tests defined, see TestNumber below)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#if !defined(P1BTESTS) || (defined(CHANGED) && defined(THREADS)) //For proj1b only compile test code if CHANGED and THREADS are defined

#include "hashchain.h"
#include <iostream>
#include <cstring>
#ifdef P1_RWLOCK
#include "rwlock.h"
#endif

#ifndef NACHOS
#include <stdio.h> //proj1a
#include <pthread.h>
#include <cstdlib>
#include <string>
#else
#include "copyright.h" //proj1b
#include "system.h"
#include "synch.h"
#endif

//use this to skip validation of results (may be easier to measure performance)
//#define NOVALIDATE 1

using namespace std;

void usage(const char *prog) { //print usage
    printf("\n Usage %s <number of threads> <num keys> <testnumber> <num iterations>\n",prog);
}

/* Test parameters */
int NumThreads = 20; //number of threads used in testing.
int NumKeys=100; //number of keys used in testing. note that for some of the predefined tests NumKeys must be >=5*NumThreads for correct testing
int NumIterations=2; //number of times to cycle through the active tests
int TestWidth=4; //width in keys of current test (default=4, larger means more contention and each thread performs more operations)
int *return_codes = NULL;

const int TABLE_SIZE=128; //this should match the table size from hashchain.h/cc (used for TEST_RWHIT)


//TLock and TSem are just used by the test rig to do synchronization outside of the code under test (for both NACHOS and pthreads)
//DO NOT MODIFY
#ifdef NACHOS
#define YIELD() currentThread->Yield()
class TLock {
  Semaphore _sem;
  public:
  TLock() : _sem((char*)"TLock semaphore",1) { }
  ~TLock() { }
  void lock() {
    _sem.P();
  }
  void unlock() {
    _sem.V();
  }
};
class TSem {
  Semaphore _sem;
  public:
  TSem(int init) : _sem((char*)"TSem semaphore",init) { }
  ~TSem() { }
  void up() {
    _sem.V();
  }
  void down() {
    _sem.P();
  }
};
#else
#define YIELD() pthread_yield()
class TLock {
  pthread_mutex_t _lock;
  public:
  TLock() {
    pthread_mutex_init(&_lock,NULL);
  }
  ~TLock() {
    pthread_mutex_destroy(&_lock);
  }
  void lock() {
    pthread_mutex_lock(&_lock);
  }
  void unlock() {
    pthread_mutex_unlock(&_lock);
  }
};
class TSem {
  sem_t _sem;
  public:
  TSem(int init) {
    sem_init(&_sem,0,init);
  }
  ~TSem() {
    sem_destroy(&_sem);
  }
  void up() {
    sem_post(&_sem);
  }
  void down() {
    sem_wait(&_sem);
  }
};
pthread_t *thr;
#endif

//simple multiple-use barrier (used to keep threads in the same test) DO NOT MODIFY
class Barrier {
  private:
    TLock lock; //lock over barrier state
    TSem sem; //semaphore for releasing threads
    volatile int count; //counter on how many threads have entered barrier
    volatile int rcount; //counter on how many threads still need to exit barrier
  public:
    Barrier() : sem(0), count(0), rcount(0)  {
    }
    ~Barrier() {
    }

    void wait(int N) { //barrier that waits for N threads to enter before continuing (your responsibility to make sure all threads use same N for same barrier)
      lock.lock();
      while(rcount) { //if other threads are still waiting to get out of the previous barrier, yield until they are all out
        lock.unlock();
        YIELD();
        lock.lock();
      }
      ++count; //increment barrier count

      if (count>=N) { //if we are the last one in we get to release everyone else
        rcount=count-1; //need to release count-1 threads
        int tcount=rcount;
        count=0;
        lock.unlock();

        //now release everyone else
        for(;tcount;--tcount) { //release each other thread
          sem.up(); //trigger waiting thread
        }
      } else { //else wait to be triggered by the last one in
        lock.unlock();
        sem.down(); //wait for trigger
        lock.lock(); --rcount; lock.unlock(); //decrement rcount to set up the next barrier
      }
    }
};


//TestNumber represents the set of implemented tests
//TODO: If you want to add any tests, you should also add a corresponding enum value to TestNumber
enum TestNumber {
  ALL_TESTS=-1,
  INVALID_TEST=0,
  TEST_GET_K, //single get to center key for each thread
  TEST_PUT_K, //single put to center key for each thread
  TEST_LOCK, //test whether we can acquire/release lock
  TEST_SIGNAL, //test whether we can signal a thread waiting on a condition variable
  TEST_BCAST, //test whether we can broadcast to all waiting threads
  TEST_WAIT, //test whether we actually wait when we call Condition::Wait()
  TEST_RW_WEXCLUDE, //test whether RWLock prevents multiple writers at once
  TEST_RW_RWEXCLUDE, //test whether RWLock prevents readers and writers at the same time
  TEST_RW_MULTIPLE_R, //test whether RWLock allows multiple readers at the same time
  TEST_SEQUENCE, //basic sequence of gets, puts, and deletes
  TEST_GET, //simple read-only sequence
  TEST_PUT, //sequence of put statements (first put writes 0, second put writes correct value i)
  TEST_DELPUT, //sequence of deletes and puts
  TEST_PUTGET, //sequence of puts and gets. set up so there are write conflicts, but not on k, so we should always be the last person to write to key k
  TEST_KEYHIT, //sequences of puts/deletes for the same key
  TEST_RWHIT, //sequence of modifications that should all land in the same bucket (to hit the rwlock harder).
  TEST_INC, //sequence of increments. If we didn't miss anything, the final value of key n should be the number of threads * the number of iterations * 100
};


// testnum is set in main() for proj1a (see end of file), and in main.cc for proj1b
int testnum = TEST_SEQUENCE;

//TODO: If you add any tests, you should also add a corresponding case to get_test_name()
//  - you can use #ifdefs to conditionally enable tests
const char* get_test_name(enum TestNumber n) {
  switch(n) {
    case ALL_TESTS: return "All Tests";
    case TEST_GET_K: return "HashMap::get() single key";
    case TEST_PUT_K: return "HashMap::put() single key";
#ifdef P1BTESTS
    case TEST_LOCK: return "Lock mutual exclusion";
    case TEST_SIGNAL: return "Condition::Signal()";
    case TEST_BCAST: return "Condition::Broadcast()";
    case TEST_WAIT: return "Condition::Wait()";
#endif
#ifdef P1_RWLOCK
    case TEST_RW_WEXCLUDE: return "RWLock W mutual exclusion()";
    case TEST_RW_RWEXCLUDE: return "RWLock R+W mutual exclusion()";
    case TEST_RW_MULTIPLE_R: return "RWLock multiple readers";
#endif
    case TEST_SEQUENCE: return "simple HashMap sequence";
    case TEST_GET: return "HashMap get sequence";
    case TEST_PUT: return "HashMap put sequence";
    case TEST_DELPUT: return "HashMap del+put sequence";
    case TEST_PUTGET: return "HashMap put+get sequence";
    case TEST_KEYHIT: return "HashMap single key hitting sequence";
    case TEST_RWHIT: return "HashMap bucket hitting sequence";
    case TEST_INC: return "HashMap increment";
    default: return NULL;
  }
  return NULL; //just for complainy compilers
}

#define enumcase(val) if (!strcmp(testname,#val)) return val
//get_testnum parses a string corresponding to one of the tests into a test number
//  TODO: If you add any tests, you should also add a corresponding line to get_testnum
int get_testnum(const char *testname) {
  enumcase(ALL_TESTS);
  else enumcase(TEST_GET_K);
  else enumcase(TEST_PUT_K);
  else enumcase(TEST_LOCK);
  else enumcase(TEST_SIGNAL);
  else enumcase(TEST_BCAST);
  else enumcase(TEST_WAIT);
  else enumcase(TEST_RW_WEXCLUDE);
  else enumcase(TEST_RW_RWEXCLUDE);
  else enumcase(TEST_RW_MULTIPLE_R);
  else enumcase(TEST_SEQUENCE);
  else enumcase(TEST_GET);
  else enumcase(TEST_PUT);
  else enumcase(TEST_DELPUT);
  else enumcase(TEST_PUTGET);
  else enumcase(TEST_KEYHIT);
  else enumcase(TEST_RWHIT);
  else enumcase(TEST_INC);
  else return atoi(testname);
}

HashMap m;


#ifdef P1BTESTS
//These will be used for project 1b testing (ignore for proj1a)
Lock student_lock((char*)"test Lock"); //for testing NACHOS Lock functionality
volatile int lock_count=0; //counter used to check for mutual exclusion in Lock

Lock student_wait_lock((char*)"Condition locker"); //Lock for testing NACHOS Condition functionality

Condition student_sig_cond((char*)"test condition for signal"); //Condition variable for testing Signal
Semaphore sig_sem((char*)"signal barrier",0); //used to synchronize the Signal tests between threads

Condition student_bcast_cond((char*)"test condition for bcast"); //Condition variable for for testing Broadcast
Semaphore bcast_sem((char*)"broadcast barrier",0); //used to synchronize the Brodcast tests between threads

Condition student_wait_cond((char*)"test condition for wait"); //Condition variable for testing Wait (ensuring we wait, and can eventually be released)
Semaphore wait_sem((char*)"wait barrier",0); //used to synchronize Wait() tests
volatile int wait_count=0; //counter used to check for mutual exclusion in Wait()
#endif

#ifdef P1_RWLOCK
RWLock rwlock; //for testing RWLock
int rw_w_count=0; //W mutual exclusion test counter
int rw_rw_count=0; //R+W mutual exclusion test counter
int rw_r_count=0; //R mutual exclusion test counter
#endif




Barrier test_barrier; //for synchronizing tests
Barrier iter_barrier; //for synchronizing test iterations


/* Each thread will use this function to access the hashtable. You can use
 * this to test your locking mechanism used to protect the hash table */
#ifdef NACHOS
void tfunc(int id){ //proj1b define tfunc() to match NACHOS Fork
#else
void *tfunc(void *arg){ //for proj1a define tfunc() to match pthread_create
  long id= (long) arg; //thread ID from pthread arg
#endif
  int i;
  int n=NumKeys; //number of keys used
  int k=(id * n) / NumThreads; //unique key for this thread (for tests that use a per-thread key)
  int w=TestWidth; //width in keys of current test (larger means more contention and each thread performs more operations)
  int err=0; //error flag
  int t; //temp var
  //Now we run through all the tests
  //  - if we are in ALL_TESTS then each case should drop through to the next, otherwise it should break after each test
  //  - you should include test_barrier.wait(NumThreads) before each test to make sure all threads are in the same test at the same time (at least if more than one test is running)
  switch(testnum) {
    case ALL_TESTS: //run through all tests
    case TEST_GET_K: //single get to unique key for each thread
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      if (m.get(k) != k) err=1;
      if (testnum != ALL_TESTS) break;
    case TEST_PUT_K: //single put to unique key for each thread
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      m.put(k,k);
      if (testnum != ALL_TESTS) break;
#ifdef P1BTESTS
    //This is all test code that will be used for project 1b
    case TEST_LOCK: //test whether lock enforces mutual exclusion
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      student_lock.Acquire();
      t=lock_count;
      YIELD(); //yield to increase chances of failing if we don't have proper locking (ensures even with cooperative scheduling the increment is unsafe)
      lock_count=t+1;
      student_lock.Release();
      if (testnum != ALL_TESTS) break;
    case TEST_SIGNAL: //test whether we can signal a thread waiting on a condition variable
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      if (id == 0) { //thread zero does the signalling (signals once for each thread)
        for(int i=1; i<NumThreads;++i) {
          sig_sem.P(); //don't acquire lock and signal until there is a thread waiting on the Condition
          student_wait_lock.Acquire();
          student_sig_cond.Signal(&student_wait_lock);
          student_wait_lock.Release();
        }
      } else { //remaining threads lock,inc semaphore, wait for signal, and unlock
        student_wait_lock.Acquire();
        sig_sem.V(); //allow thread 0 to take lock and signal once
        student_sig_cond.Wait(&student_wait_lock);
        student_wait_lock.Release();
      }
      if (testnum != ALL_TESTS) break;
    case TEST_BCAST: //test whether we can broadcast to all waiting threads
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      if (id == 0) { //thread zero does the signalling (waits for all threads to start waiting, then broadcasts)
        for(int i=1; i<NumThreads;++i) {
          bcast_sem.P();
        }
        student_wait_lock.Acquire();
        student_bcast_cond.Broadcast(&student_wait_lock);
        student_wait_lock.Release();
      } else { //remaining threads lock,inc semaphore, wait for signal, and unlock
        student_wait_lock.Acquire();
        bcast_sem.V();
        student_bcast_cond.Wait(&student_wait_lock);
        student_wait_lock.Release();
      }
      if (testnum != ALL_TESTS) break;
    case TEST_WAIT: //test whether we actually wait in Condition::Wait
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      if (id == 0) { //thread zero does the signalling (waits for all threads to start waiting, then verifies wait and broadcasts)
        for(int i=1; i<NumThreads;++i) { //wait for all threads to get into wait state
          wait_sem.P();
        }
        student_wait_lock.Acquire();
        if (wait_count != 0) { //if this is >0 then at least 1 thread made it past call to Wait() before we called Broadcast()
          err |= 1;
          cout << "Condition::Wait test failed\n";
        }
        student_wait_cond.Broadcast(&student_wait_lock);
        student_wait_lock.Release();
      } else { //remaining threads lock,inc semaphore, wait for signal, and unlock
        student_wait_lock.Acquire();
        wait_sem.V();
        student_wait_cond.Wait(&student_wait_lock);
        wait_count++;
        student_wait_lock.Release();
      }
      if (testnum != ALL_TESTS) break;
#endif
#ifdef P1_RWLOCK
    //These tests are only used when we are compiling for P1_RWLOCK tests
    case TEST_RW_WEXCLUDE: //test whether RWLock prevents multiple writers at once
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      rwlock.startWrite();
      t=rw_w_count;
      YIELD();
      rw_w_count=t+1;
      rwlock.doneWrite();
      if (testnum != ALL_TESTS) break;
    case TEST_RW_RWEXCLUDE: //test whether RWLock prevents readers and writers at the same time
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      if(id==0) { //thread 0 grabs read lock once for each thread and increments
        for(i=0;i<NumThreads;++i) {
          rwlock.startRead();
          t=rw_rw_count;
          YIELD();
          rw_rw_count=t+1;
          rwlock.doneRead();
        }
      }
      //each thread grabs write lock and increments once (including thread 0)
      rwlock.startWrite();
      t=rw_rw_count;
      YIELD();
      rw_rw_count=t+1;
      rwlock.doneWrite();
      if (testnum != ALL_TESTS) break;
    case TEST_RW_MULTIPLE_R: //test whether RWLock allows multiple readers at the same time
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      rwlock.startRead();
      //for P1_RWLOCK RWLock MUST allow multiple readers (so we barrier inside the read lock)
      test_barrier.wait(NumThreads);
      rwlock.doneRead();
      if (testnum != ALL_TESTS) break;
#endif
    case TEST_SEQUENCE: //basic sequence of gets, puts, and deletes
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      for (i=0; i<n; i++) { m.get(i); }
      for (i=k-w; i<=k+w; i++) { if(i<n && i>=0) m.put(i,i); }
      if(m.get(k)==-1) err=1;
      m.remove(k);
      for (i=0; i<n; i++) { m.get(i); }
      m.put(k,k);
      for (i=0; i<n; i++) { m.get(i); }
      if(m.get(k)==-1) err=1;
      if (testnum != ALL_TESTS) break;
    case TEST_GET: //simple read-only sequence
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      for (i=k-w; i<=k+w; i++) { if(i<n && i>=0) err |= m.get(i) != i; }
      if (testnum != ALL_TESTS) break;
    case TEST_PUT: //sequence of put statements (first put writes 0, second put writes i)
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      for (i=k-w; i<=k+w; i++) { if(i<n && i>=0) m.put(i,0); }
      for (i=k-w; i<=k+w; i++) { if(i<n && i>=0) m.put(i,i); }
      if (testnum != ALL_TESTS) break;
    case TEST_DELPUT: //sequence of deletes and puts
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      for (i=k-w; i<=k+w; i++)
        if(i<n && i>=0) m.remove(i);
      for (i=k-w; i<=k+w; i++)
        if(i<n && i>=0) m.put(i,i);
      if (testnum != ALL_TESTS) break;
    case TEST_PUTGET: //sequence of puts and gets. set up so there are write conflicts, but not on k, so we should always be the last person to write to key k
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      for (i=k-w; i<=k+w; i++) if(i<n && i>=0) m.put(i,0);
      err |= m.get(k) != 0;
      for (i=k-w; i<=k+w; i++) if(i<n && i>=0) m.put(i,1);
      err |= m.get(k) != 1;
      for (i=k-w; i<=k+w; i++) if(i<n && i>=0) m.put(i,i+1);
      err |= m.get(k) != k+1;
      for (i=k-w; i<=k+w; i++) if(i<n && i>=0) m.put(i,k+3);
      err |= m.get(k) != k+3;
      for (i=k-w; i<=k+w; i++) if(i<n && i>=0) m.put(i,i);
      err |= m.get(k) != k;
      if (testnum != ALL_TESTS) break;
    case TEST_KEYHIT: //sequences of puts/deletes for the same key
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      for(i=0;i<100;++i) {
        m.put(n+1,0);
        m.remove(n+1);
        m.put(n+1,1);
        m.remove(n+1);
      }
      if (testnum != ALL_TESTS) break;
    case TEST_RWHIT: //sequence of modifications that should all land in the same bucket (to hit the rwlock harder).
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      for(i=0,t=n+1;i<100;++i,t+=TABLE_SIZE) { m.put(t,t); }
      for(i=0,t=n+1;i<100;++i,t+=TABLE_SIZE) { m.remove(t); }
      for(i=0,t=n+1;i<100;++i,t+=TABLE_SIZE) { m.put(t,t); }
      for(i=0,t=n+1;i<100;++i,t+=TABLE_SIZE) { m.remove(t); }
      if (testnum != ALL_TESTS) break;
    case TEST_INC: //sequence of increments. If we didn't miss anything, the final value of key n should be the number of threads * the number of iterations * 100
      if (testnum == ALL_TESTS) test_barrier.wait(NumThreads); //barrier before test if we are running multiple tests
      for (i=0;i<100;++i) { m.increment(n,1); }
      break;
      //TODO: you can define additional tests you want to run here
    default: //otherwise we tried to run a disabled test
      err=-1;
  }

  return_codes[id] = err;
  iter_barrier.wait(NumThreads+1); //synchronize iterations between threads
#ifdef NACHOS
  if (NumThreads > 0) currentThread->Finish(); //for NACHOS call Finish() on the thread
#else
  return 0; //for proj1b return from tfunc
#endif
}

//ThreadTester either forks a NACHOS thread or creates a pthread depending on whether we are in NACHOS
#ifdef NACHOS
  void ThreadTester(int childThreadNum)
  {
    DEBUG('t', (char*)"Entering ThreadTest1");
    // Prepare a new thread to be forked
    Thread *t = new Thread((char*)"forked thread");
    // 'Fork' the new thread, putting it on the ready queue, about to run tfunc
    t->Fork(tfunc, childThreadNum);
  }
#else
  void ThreadTester(int childThreadNum)
  {
    pthread_create(&thr[childThreadNum],NULL, tfunc, (void *)((long)childThreadNum));
  }
#endif


#ifdef NACHOS
//in NACHOS ThreadTest is defined to return void in main.cc
void ThreadTest(void)
{
  if (return_codes == NULL) return_codes = new int[NumThreads];
  if(NumThreads < 0) {
    cout << "Num threads must be >= 0\n";
    return;
  } else if (NumKeys < 5*NumThreads) {
    cout << "Num threads must be >= 5*num iterations for all tests to pass\n";
    return;
  } else if (NumIterations < 1) {
    cout << "Num iterations must be >= 1\n";
    return;
  }
  const char *testname = get_test_name((enum TestNumber)testnum);
  if (testname) {
    cout << "Running Test: '" << testname << "' in " << NumThreads << " threads for " << NumIterations << " iterations with " << NumKeys << " Keys\n";
  } else {
    cout << "Invalid Test Number\n";
    return;
  }
#else
//in pthreads ThreadTest returns an int so we can get our return code for main()
int ThreadTest(void)
{
  if (return_codes == NULL) return_codes = new int[NumThreads];
  if(NumThreads < 0) {
    cout << "Num threads must be >= 0\n";
    return -1;
  } else if (NumKeys < 5*NumThreads) {
    cout << "Num threads must be >= 5*num iterations for all tests to pass\n";
    return -1;
  } else if (NumIterations < 1) {
    cout << "Num iterations must be >= 1\n";
    return -1;
  }
  const char *testname = get_test_name((enum TestNumber)testnum);
  if (testname) {
    cout << "Running Test: '" << testname << "' in " << NumThreads << " threads for " << NumIterations << " iterations with " << NumKeys << " Keys\n";
  } else {
    cout << "Invalid Test Number\n";
    return -1;
  }
#endif

  int err=0;
  int errflag=0;
  int k;
  int val;

  // initialize test keys
  for (int i=0; i<NumKeys; i++) {
    m.put(i,i);
  }

  m.put(NumKeys,0); //initialize counter for increment()

  //loop through the test NumIterations times
  for(int iter = 0; iter < NumIterations; ++iter) {
    //loop initialization
#ifdef P1BTESTS
    wait_count=0; //proj1b testing of Wait()
#endif
    

    // Spawn N child threads to run tests
    for(int i = 0; i < NumThreads; i++) {
      ThreadTester(i); // Pass the child thread's id as arg
    }
    //wait for N child threads to finish (the +1 is for the this call to ThreadTest())
    iter_barrier.wait(NumThreads+1);
#ifndef NACHOS
    //join the threads if using pthreads
    for(int i=0;i<NumThreads;i++) {
      void* retval = 0;
      pthread_join(thr[i],&retval);
    }
#endif
    //check return code from each thread (they should all be zero)
    for(int i = 0; i < NumThreads; i++) {
      if (return_codes[i]) err++;
    }
    if (errflag) {
      cout << err << " tests returned failure codes for iteration " << iter << endl;
      errflag|=err;
      err=0;
      break; //once we fail, don't keep going (but still print the final key values)
    }

#ifndef NOVALIDATE
    for (k=0; k<NumKeys; ++k) { //verify key-values (all tests should leave each key with hastable[k] = k for this to work)
      errflag |= m.get(k) != k;
    }
    if(errflag) {
      cout << "Failed to find  some elements or value was incorrect after iteration " << iter <<endl;
      break;
    }
#endif
    errflag |= err;
    err=0;
  }

#ifndef NOVALIDATE
  //all tests leave every key in a well defined state (get(i) == i) so we can check if anything broke here
  for (k=0; k<NumKeys; ++k) {
    val=m.get(k);
    err |= val != k;
    cout << " " << val;
    if (0==((k+1)%25) || k==(NumKeys-1)) cout << endl;
  }
  if(err)
    cout << "Failed to find  some elements or value was incorrect"<<endl;
  errflag |= err;
  err=0;

  if (testnum == TEST_INC || testnum == ALL_TESTS) {
    const int correct_sum = NumIterations*NumThreads*100;
    val=m.get(NumKeys);
    if (val != correct_sum) {
      errflag |= 1;
      cout << "HashMap increment resulted in wrong value. sum should be " << correct_sum << " but instead got " << val << endl;
    } else {
      cout << "HashMap increment resulted in correct value of " << correct_sum << endl;
    }
  }

#ifdef P1BTESTS
  if (testnum == TEST_LOCK || testnum == ALL_TESTS) { //for proj1b
    const int correct_sum = NumIterations*NumThreads;
    if (lock_count != correct_sum) {
      errflag |= 1;
      cout << "Lock did not enforce mutual exclusion. sum should be " << correct_sum << " but instead got " << lock_count << endl;
    } else {
      cout << "Lock increment resulted in correct value of " << correct_sum << endl;
    }
  }
#endif

#ifdef P1_RWLOCK
  if (testnum == TEST_RW_WEXCLUDE || testnum == ALL_TESTS) {
    const int correct_sum = NumIterations*NumThreads;
    if (rw_w_count != correct_sum) {
      errflag |= 1;
      cout << "RWLock did not enforce mutual exclusion between writers. sum should be " << correct_sum << " but instead got " << rw_w_count << endl;
    } else {
      cout << "RWLock W increment resulted in correct value of " << correct_sum << endl;
    }
  }
  if (testnum == TEST_RW_RWEXCLUDE || testnum == ALL_TESTS) {
    const int correct_sum = NumIterations*NumThreads*2;
    if (rw_rw_count != correct_sum) {
      errflag |= 1;
      cout << "RWLock did not enforce mutual exclusion between readers and writers. sum should be " << correct_sum << " but instead got " << rw_rw_count << endl;
    } else {
      cout << "RWLock RW increment resulted in correct value of " << correct_sum << endl;
    }
  }
#endif
#else
  cout << "Skipping result validation...\n";
#endif

  if (!errflag) //if no error flags were set
    cout << "All tests passed. This doesn't mean everything is correct, just that no bugs were uncovered by the test(s) that were run."<<endl;
  else
    cout << "One or more tests failed. See output above for hints"<<endl;
#ifndef NACHOS
  return errflag; //only return for pthreads (for NACHOS this function is void)
#endif
}
#endif //for proj1b

#ifndef NACHOS
//if we aren't running this in NACHOS, we use this main() function (otherwise nachos has it's own main() function in main.cc)
int main(int argc, char*argv[])
{
  if(argc != 5) { //correct num args
    usage(argv[0]);
    return 1;

  }  

  //parse args
  NumThreads = atoi(argv[1]);
  NumKeys = atoi(argv[2]);
  testnum = get_testnum(argv[3]);
  NumIterations = atoi(argv[4]);

  if(NumThreads < 0) {
    cout << "Num threads must be >= 0\n";
    usage(argv[0]);
    return -1;
    //return;
  } else if (NumKeys < 5*NumThreads) {
    cout << "Num threads must be >= 5*num iterations for all tests to pass\n";
    usage(argv[0]);
    return -1;
    //return;
  } else if (NumIterations < 1) {
    cout << "Num iterations must be >= 1\n";
    usage(argv[0]);
    return -1;
    //return;
  }
  const char *testname = get_test_name((enum TestNumber)testnum);
  if (testname) {
  } else {
    cout << "Invalid Test Number\n";
    usage(argv[0]);
    return -1;
    //return;
  }
  thr = (pthread_t *)malloc(sizeof(pthread_t) * NumThreads); //allocate array of threads

  return ThreadTest(); //call threadtest
}
#endif
