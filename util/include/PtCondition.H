#ifndef PtCondition_H
#define PtCondition_H

#include "StorageMgr_Exceptions.H"
#include <exception>

#include <pthread.h>

class PtMutex;

class PtCondition
{
public:
  PtCondition()
    throw (std::exception);

  virtual ~PtCondition()
    throw (std::exception);

  void signal()
    throw (std::exception);

  void broadcast()
    throw (std::exception);
private:
  friend class PtMutex;

  pthread_cond_t _cond;
};

#endif
