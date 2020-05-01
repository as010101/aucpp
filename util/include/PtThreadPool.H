#ifndef PtThreadPool_H
#define PtThreadPool_H

#include "StorageMgr_Exceptions.H"
#include "ClosableFifoQueue.H"
#include "Runnable.H"
#include "PtMutex.H"
#include <pthread.h>
#include <queue>
#include <set>
#include <string>

class PtThreadPool
{
public:
  // The thread pool will start with minThreads, but will launch up to a 
  // total of maxThreads threads. After a Runnable has been executed by one
  // of this pool's threads, the address of the Runnable is inserted into
  // the 'executedRunnables' queue. When the 'executedRunnables' queue 
  // becomes 'closed', then this PtThreadPool has fully quiesced.
  PtThreadPool(int minThreads,
	       int maxThreads, 
	       ClosableFifoQueue<Runnable*> & executedRunnables)
    throw (std::exception);

  virtual ~PtThreadPool();

  // Schedules the runnable for execution by this thread pool. Between the
  // submission of a runnable, and its re-appearance of its address in the 
  // 'executedRunnables' queue, the caller may not call any method on the
  // Runnable object and may not delete it.
  //
  // Runnables are assigned to threads in this pool in a FIFO manner.
  void schedule(Runnable * pRunnable)
    throw (std::exception);

  // Disables future invocations of the 'schedule(...)' method. The next
  // time that the number of Runnables scheduled for this pool goes down
  // to zero, this pool will call the 'close()' method on the 
  // 'execeutedRunnables' queue.
  void quiesce()
    throw (std::exception);

private:
  static void *threadFunc(void *pPool);

  // Creates a server thread, which then enters the cycle of awaiting 
  // the availability if an object in _scheduledRunnables.
  void createServerThread()
    throw (std::exception);

  // This is just an object-oriented version of threadFunc(...).
  void serverThreadMethod();

  ClosableFifoQueue<Runnable*>   _scheduledRunnables;
  ClosableFifoQueue<Runnable*> & _executedRunnables;

  int _minThreads;
  int _maxThreads;
  int _numCurrentThreads;
  int _numPendingRunnables;
  //  bool _quiescing;

  PtMutex _mtx; // General guardian for this class' methods.

  set<pthread_t> _threads;

  // Posted once all server threads have finished any interesting work they're 
  // going to do, *and* aren't going to make any more attempts to acquire _mtx.
  BinarySem _allThreadsQuiescing;
};

#endif
