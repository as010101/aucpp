#include <iostream>
#include <ClosableFifoQueue.H>
#include <pthread.h>
#include <unistd.h>
#include <sstream>

using namespace std;

//==================================================================

pthread_mutex_t ioMutex = PTHREAD_MUTEX_INITIALIZER;

void printmsg(string s)
{
  pthread_mutex_lock(& ioMutex);
  cout << s << endl;
  pthread_mutex_unlock(& ioMutex);
}

//==================================================================

void *pushFunc(void *p)
{
  printmsg("pushFunc: Entering");

  ClosableFifoQueue<string> * pQ = 
    reinterpret_cast<ClosableFifoQueue<string> *>(p);

  for (int i = 1; i <= 5; i++)
    {
      ostringstream strData, strMsg;
      strData << "A String #" << i;

      strMsg << "pushFunc: About to push \"" << strData.str() << "\"";
      printmsg(strMsg.str());

      pQ->enqueue(strData.str());
      sleep(1);
    }

  pQ->close();

  printmsg("pushFunc: Exiting");
  return NULL;
}

//==================================================================

void *pullFunc(void *p)
{
  printmsg("pullFunc: Entering");

  ClosableFifoQueue<string> * pQ = 
    reinterpret_cast<ClosableFifoQueue<string> *>(p);

  bool done = false;
  while (!done)
    {
      printmsg("pullFunc: About to call dequeueBlocking()");

      try
	{
	  string r = pQ->dequeueBlocking();

	  ostringstream str;
	  str << "pullFunc: Just pulled \"" << r << "\"";
	  printmsg(str.str());
	}
      catch(SmClosedException &e)
	{
	  string msg = "pullFunc: Caught ClosedException: ";
	  msg += e.getDescription();
	  printmsg(msg);
	  done = true;
	}
      catch(SmException &e)
	{
	  string msg = "pullFunc: Caught Exception: ";
	  msg += e.getDescription();
	  printmsg(msg);
	  done = true;
	}
    }

  printmsg("pullFunc: Exiting");
  return NULL;
}

//==================================================================

int main()
{
  ClosableFifoQueue<string> p(NULL, NULL, NULL);
  pthread_t push_pid, pull_pid;

  pthread_create(& pull_pid, NULL, pullFunc, &p);
  pthread_create(& push_pid, NULL, pushFunc, &p);
  pthread_join(pull_pid, NULL);
  printmsg("main: Done");
}
