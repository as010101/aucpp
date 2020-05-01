#include <PtThreadPool.H>
#include <util.H>
#include <iostream>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>


//===============================================================================

void printUsage()
{
  cout << "Usage: testPtTheadPool <min-threads> <max-threads> <num-runnables>" << endl;
}

//===============================================================================

class GoodRunnable : public Runnable
{
public:
  GoodRunnable(int id, int iterations, int delay);

  void run() throw();

private:
  int _id;
  int _iterations;
  int _delay;

};

GoodRunnable::GoodRunnable(int id, int iterations, int delay)
{
  _id = id;
  _iterations = iterations;
  _delay = delay;
}

void GoodRunnable::run() throw()
{
  try
    {
      cout << "Starting: " << _id << endl;
      for (int i = 0; i < _iterations; i++)
	{
	  sleep(_delay);
	  cout << "Heartbeat: " << _id << endl;
	}

      cout << "Ending: " << _id << endl;
    }
  catch (std::exception &e)
    {
      cloneAndSetRunException(e);
    }
  catch (...)
    {
      cout << "GoodRunnable::run(): Caught totally unexpected object" << endl;
      abort();
    }
}

//===============================================================================

class BadRunnable : public Runnable
{
public:
  BadRunnable(int id, int delay);

  void run() throw();

private:
  int _id;
  int _delay;

};

BadRunnable::BadRunnable(int id, int delay)
{
  _id = id;
  _delay = delay;
}

void BadRunnable::run() throw()
{
  try
    {
      cout << "BadRunnable Starting: " << _id << endl;
      sleep(_delay);
      throw SmException(__FILE__, __LINE__, "Intentional exception");
      cout << "BadRunnable Ending: " << _id << endl;
    }
  catch (std::exception &e)
    {
      cloneAndSetRunException(e);
    }
  catch (...)
    {
      cout << "GoodRunnable::run(): Caught totally unexpected object" << endl;
      abort();
    }
}

//===============================================================================

int main(int argc, char** argv)
{
  if (argc != 4)
    {
      printUsage();
      exit(1);
    }

  int minThreads   = atoi(argv[1]);
  int maxThreads   = atoi(argv[2]);
  int numRunnables = atoi(argv[3]);
    
  try
    {
      ClosableFifoQueue<Runnable*> qResults(NULL, NULL, NULL);
      PtThreadPool aPool(minThreads, maxThreads, qResults);

      for (int i = 0; i < numRunnables; i++)
	{
	  Runnable *pRunnable;

	  if ((i % 2) == 0)
	    {
	      pRunnable = new GoodRunnable(i, 3, 1);
	      cout << "Created GoodRunnable: " << pRunnable << endl;
	      aPool.schedule(pRunnable);
	    }
	  else
	    {
	      pRunnable = new BadRunnable(i, 1);
	      cout << "Created BadRunnable: " << pRunnable << endl;
	      aPool.schedule(pRunnable);
	    }
	}
      cout << endl;

      sleep(5);

      aPool.quiesce();

      bool done = false;
      while (! done)
	{
	  try
	    {
	      cout << "About to call qResults.dequeueBlocking()" << endl;
	      Runnable * pResult = qResults.dequeueBlocking();
	      cout << "Returned from call qResults.dequeueBlocking()" << endl << endl;

	      cout << "qResults: Just returned object: " << pResult << endl;

	      std::exception * pException = pResult->getRunException();
	      if (pException == NULL)
		{
		  cout << "qResults: getRunException returned NULL" << endl;
		}
	      else
		{
		  cout << "qResults: getRunException exception had description: " 
		       << pException->what() << endl;
		}

	      delete pResult;
	    }
	  catch (SmClosedException e)
	    {
	      cout << "* qResults QUEUE IS CLOSED AND EMPTY *" << endl;
	      done = true;
	    }

	  cout << endl;
	}
    }
  catch (std::exception &e)
    {
      cerr << "Exception thrown:" << endl << e.what() << endl;
      abort();
    }
  catch (...)
    {
      cout << "main(...): Some other exception caught." << endl;
      abort();
    }
}

//===============================================================================
