#ifndef Runnable_H
#define Runnable_H

#include "StorageMgr_Exceptions.H"
#include <string>

// This class is not designed to gracefully handle concurrently multithreaded
// access, so be careful.
class Runnable
{
public:
  Runnable();
  Runnable(const Runnable & rhs);
  virtual ~Runnable();
  Runnable & operator=(const Runnable & rhs);

  // Returns a pointer to the exception, if there is one, that was thrown by the
  // Runanble's 'run()' method's most recent execution in a PtThreadPool.

  // The pointer is only valid until any of the following events:
  // (1) the next invocation of this class' 'deleteRunException()' method, or
  // (2) this Runnable is once again submitted for execution into a PtThreadPool, or
  // (3) this Runnable is deleted.
  exception *getRunException() const;

  void deleteRunException(); // Invoking this will delete the current RunExcption from the
  // heap. Note: So will another invocation of 'cloneAndSetRunException(...)'.

  // This method is invoked when this object is executed in a thread.
  virtual void run() throw() = 0;

  void setRunException(exception *pException);

  // Clones the specified exception onto the heap, and then makes that clone be
  // this Runnable's current RunException.
  void cloneAndSetRunException(const std::exception &e);

private:

  exception *_lastRunException;
};

#endif
