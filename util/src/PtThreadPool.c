#include <PtThreadPool.H>
#include <LockHolder.H>
#include <assert.h>
#include <iostream>


//===============================================================================

PtThreadPool::PtThreadPool(int minThreads,
			   int maxThreads, 
			   ClosableFifoQueue<Runnable*> & executedRunnables)
  throw (std::exception)
  : _scheduledRunnables(NULL, NULL, NULL, "PtThreadPool._scheduledRunnables"),
    _executedRunnables(executedRunnables),
    _minThreads(minThreads),
    _maxThreads(maxThreads),
    _numCurrentThreads(0),
    _numPendingRunnables(0) 
  //    _quiescing(false)
{
  if (minThreads < 0)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "minThreads < 0");
    }

  if (minThreads > maxThreads)
    {
      throw SmIllegalParameterValueException(__FILE__, __LINE__, "minThreads > maxThreads");
    }

  for (int i = 0; i < _minThreads; i++)
    {
      createServerThread();
    }
}

//===============================================================================

PtThreadPool::~PtThreadPool()
{
  _allThreadsQuiescing.awaitPostThenClear();

  {
    LockHolder mutexHolder(_mtx);

    set<pthread_t>::iterator pos = _threads.begin();
    while (pos != _threads.end())
      {
	int rc = pthread_join(*pos, NULL);
	if (rc != 0)
	  {
	    cerr << "PtThreadPool::~PtThreadPool(): Problem joining a worker thread" << endl;
	    assert(false);
	  }

	++pos;
      }
  }
}

//===============================================================================
  
void PtThreadPool::schedule(Runnable * pRunnable)
  throw (std::exception)
{
  LockHolder mutexHolder(_mtx);

  if (_scheduledRunnables.isClosed())
    {
      throw SmException(__FILE__, __LINE__, "quiece() has already been called");
    }
  
  pRunnable->setRunException(NULL);
  _numPendingRunnables++;
  _scheduledRunnables.enqueue(pRunnable);

  if ((_numCurrentThreads < _numPendingRunnables) && 
      (_numCurrentThreads < _maxThreads))
    {
      createServerThread();
    }
}

//===============================================================================

void PtThreadPool::quiesce()
  throw (std::exception)
{
  LockHolder mutexHolder(_mtx);

  if (_scheduledRunnables.isClosed())
    {
      throw SmException(__FILE__, __LINE__, "quiece() has already been called");
    }

  _scheduledRunnables.close();
}

//===============================================================================

void PtThreadPool::createServerThread()
  throw (std::exception)
{
  pthread_t aThread;
  int rc = pthread_create(& aThread, NULL, PtThreadPool::threadFunc, this);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, "Failed call to pthread_create(...)");
    }

  _threads.insert(aThread);
}

//===============================================================================

void *PtThreadPool::threadFunc(void *p)
{
  assert(p != NULL);
  PtThreadPool *pPool = reinterpret_cast<PtThreadPool*>(p);
  pPool->serverThreadMethod();
  return NULL;
}

//===============================================================================

void PtThreadPool::serverThreadMethod()
{
  try
    {
      // These are safe to test outside of the mutex. We test them now to avoid
      // a deadlock problem between this method and ~PtThreadPool()....
      if (_scheduledRunnables.isClosed() &&
	  _scheduledRunnables.isEmpty())
	{
	  return;
	}

      {
	LockHolder mutexHolder(_mtx);
	_numCurrentThreads++;
      }
      
      bool poolShutdown = false;
      
      while (! poolShutdown)
	{
	  Runnable * pRunnable = NULL;
	  
	  try
	    {
	      pRunnable = _scheduledRunnables.dequeueBlocking();
	      assert(pRunnable != NULL);
	      pRunnable->run();
	    }
	  catch (SmClosedException e)
	    {
	      poolShutdown = true;  
	    }
	  
	  if (! poolShutdown)
	    {
	      _executedRunnables.enqueue(pRunnable);
	      LockHolder mutexHolder(_mtx);
	      _numPendingRunnables--;
	    }
	}
      
      {
	LockHolder mutexHolder(_mtx);
	
	_numCurrentThreads--;
	
	if ((_numCurrentThreads == 0) && 
	    _scheduledRunnables.isClosed() &&
	    _scheduledRunnables.isEmpty())
	  {
	    _executedRunnables.close();
	    _allThreadsQuiescing.post();
	  }
      }
    }
  catch (std::exception e)
    {
      cerr << "PtThreadPool::serverThreadMethod(): Caught std::exception:" << endl
	   << e.what() << endl;
      abort();
    }
  catch (...)
    {
      cerr << "PtThreadPool::serverThreadMethod(): Caught an exception other than a std::exception." << endl;
      abort();
    }
}

//===============================================================================

  
