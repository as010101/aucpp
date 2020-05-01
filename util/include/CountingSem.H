#ifndef CountingSem_H
#define CountingSem_H

#include "StorageMgr_Exceptions.H"
#include "PtMutex.H"

class CountingSem
{
public:
  CountingSem(int maxConcurrentHoldings)
    throw (std::exception);

  virtual ~CountingSem();

  /*
    Changes the number of resource intances in this semaphore. If the new
    value is < the number currently held, then no change is made and an exception
    is thrown.
   */
  void setMaxConcurrentHoldings(int maxConcurrentHoldings)
    throw (std::exception);

  /*
    Acquires an instance of the specified resource. If successful, returns 'true'.
    If failed because of a resource shortage, returns 'false'.
   */
  bool acquireNonBlocking()
    throw (std::exception);

  void release()
    throw (std::exception);

private:
  PtMutex _mtx;
  int _maxConcurrentHoldings;
  int _currentHoldings;
};


#endif
