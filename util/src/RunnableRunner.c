#include <RunnableRunner.H>
#include <pthread.h>
#include <iostream>

//===============================================================================

RunnableRunner::RunnableRunner(Runnable & aRunnable)
  throw (std::exception) :
  _pRunnable(& aRunnable),
  _joinCalled(false)
{
  int rc = pthread_create(& _tid, NULL, RunnableRunner::threadFunc, this);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, "Failed call to pthread_create(...)");
    }
}

//===============================================================================

RunnableRunner::~RunnableRunner()
{
  if (! _joinCalled)
    {
      cerr << "RunnableRunner::~RunnableRunner() : Warning: Object deleted before join() was called." 
	   << endl;
    }
}

//===============================================================================
  
Runnable * RunnableRunner::getRunnable()
  throw (std::exception)
{
  return _pRunnable;
}

//===============================================================================

void RunnableRunner::join()
  throw (std::exception)
{
  if (_joinCalled)
    {
      throw SmException(__FILE__, __LINE__, "join() has already been called");
    }

  int rc = pthread_join(_tid, NULL);
  if (rc != 0)
    {
      throw SmException(__FILE__, __LINE__, "Failed call to pthread_join(...)");
    }

  _joinCalled = true;
}

//===============================================================================

void *RunnableRunner::threadFunc(void *pRunnableRunner)
{
  assert(pRunnableRunner != NULL);

  try
    {
      RunnableRunner *pRunner = reinterpret_cast<RunnableRunner*>(pRunnableRunner);
      pRunner->_pRunnable->deleteRunException();
      pRunner->_pRunnable->run();
    }
  catch (std::exception e)
    {
      cerr << "RunnableRunner::threadFunc(): Caught std::exception:" << endl
	   << e.what() << endl;
      assert(false);
    }
  catch (...)
    {
      cerr << "RunnableRunner::threadFunc(): Caught an exception other than a std::exception." << endl;
      assert(false);
    }

  return NULL;
}

//===============================================================================

RunnableRunner::RunnableRunner(const RunnableRunner & rhs)
{
  assert(false); // This method should never be called. That's why it's private.
}

//===============================================================================

RunnableRunner & RunnableRunner::operator= (const RunnableRunner & rhs)
{
  assert(false); // This method should never be called. That's why it's private.
  return *this;
}

//===============================================================================
