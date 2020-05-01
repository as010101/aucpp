#include <TraceLogger.H>
#include <iostream>
#include <Runnable.H>
#include <RunnableRunner.H>
#include <parseutil.H>
#include <sstream>
#include <sys/time.h>
#include <unistd.h>
#include <util.H>

using namespace std;

//===============================================================================

void printUsage()
{
  cout << "Usage: testTraceLogger <config-file.xml> <seconds> <thread-sleep-usec>" 
       << endl
       << endl
       << "   Note: Each of the threads uses a log group named \"thread_x\", " << endl
       << "   where x = (thread-num % 3)" << endl
       << "   (Threads are numbered 0...(n-1)), where 'n' is hard-coded in " << endl
       << "   test program" << endl << endl;
}

//===============================================================================

class LogTestRunnable : public Runnable
{
public:
  LogTestRunnable(TraceLogger & logger, 
		  size_t secondsToRun, 
		  size_t sleepUsec,
		  string threadName) :
    _logger(logger),
    _secondsToRun(secondsToRun),
    _sleepUsec(sleepUsec),
    _threadName(threadName)

  {
  }

  //-----------------------------------------------------------------------------

  virtual void run() 
    throw()
  {
    try
      {
	timeval now;
	gettimeofday(& now, NULL);

	timeval endTime = now;
	endTime.tv_sec += _secondsToRun;

	while (timevalsComp(now, endTime) != 1)
	  {
	    _logger.log(_threadName, "Hello, logger.");
	    
	    usleep(_sleepUsec);
	    gettimeofday(& now, NULL);
	  }
      }
    catch (const exception & e)
      {
	cloneAndSetRunException(const_cast<exception &>(e));
      }
  }

private:
  TraceLogger & _logger;
  size_t _secondsToRun;
  size_t _sleepUsec;
  string _threadName;
};

//===============================================================================

void test1(string propsFilename,
	   size_t secondsToRun,
	   size_t sleepUsec)
{
  cout << "***********************************************************" << endl
       << "*                     BEGINNING TEST #1                   *" << endl
       << "***********************************************************" << endl
       << endl;

  bool passingTest = true;

  PropsFile pf(propsFilename);
  TraceLogger logger(pf);

  size_t numThreads = 10;
  vector<Runnable *>       runnables;
  vector<RunnableRunner *> runners;
  
  for (size_t i = 0; i < numThreads; ++i)
    {
      ostringstream os;
      os << "thread_" << (i % 3);

      Runnable * pRunnable = new LogTestRunnable(logger, secondsToRun, sleepUsec, 
						 os.str());
      runnables.push_back(pRunnable);
      runners.push_back(new RunnableRunner( *(pRunnable)));
    }

  
  for (size_t i = 0; i < numThreads; ++i)
    {
      runners.at(i)->join();

      exception * pException = runners.at(i)->getRunnable()->getRunException();
      if (pException != NULL)
	{
	  cout << "!!! UH-OH: A log-testing thread threw an exception: " << endl
	       << pException->what() << endl << endl;
	  passingTest = false;
	}

      delete runners.at(i);
      delete runnables.at(i);
    }

  if (! passingTest)
    {
      throw SmException(__FILE__, __LINE__, "At least one test-thread had a problem.");
    }

  cout << "***********************************************************" << endl
       << "*                        PASSED TEST #1                   *" << endl
       << "***********************************************************" << endl
       << endl;
}

//===============================================================================

int main(int argc, char* argv[])
{
  if (argc != 4)
    {
      printUsage();
      return 1;
    }

  try
    {
      string propsFilename = argv[1];
      size_t secondsToRun  = stringToSize_t(argv[2]);
      size_t sleepUsec     = stringToSize_t(argv[3]);

      test1(propsFilename, secondsToRun, sleepUsec);
    }
  catch (const exception & e)
    {
      cout << "main(): Caught exception: " << e.what() << endl;
      return 1;
    }
  catch (...)
    {
      cout << "main(): Caught something not publicly derived from 'exception'" 
	   << endl;
      return 1;
    }

  return 0;
}

//===============================================================================
