#ifndef RUNNABLERUNNER_H
#define RUNNABLERUNNER_H

#include "StorageMgr_Exceptions.H"
#include "Runnable.H"
#include "PtMutex.H"

/*
  Provides for the one-time running of a Runnable.

  NOTE: THIS CLASS IS NOT SAFE FOR CONCURRENT METHOD INVOCATIONS.
 */

class RunnableRunner
{
public:
  // Launches the specified runnable in a new thread.
  RunnableRunner(Runnable & aRunnable)
    throw (std::exception);

  virtual ~RunnableRunner();
  
  // Gets the address of the  Runnable that was most recently supplied to 
  // the start(...) method.
  // Raises an exception if the start(...) method hasn't been called yet.
  Runnable * getRunnable()
    throw (std::exception);

  // Waits for the currently executing Runnable to return from its run() method.
  // Raises an exception if start(...) hasn't been called yet, or if join() has
  // already been called since the last execution of a Runnable.
  void join()
    throw (std::exception);

private:
  RunnableRunner(const RunnableRunner & rhs);
  RunnableRunner & operator= (const RunnableRunner & rhs);

  static void *threadFunc(void *pRunnableRunner);

  PtMutex _mtx;
  Runnable * _pRunnable;
  bool _joinCalled;
  pthread_t _tid;
};

#endif
