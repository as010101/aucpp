#ifndef LOCKHOLDER_H
#define LOCKHOLDER_H

#include "ILockable.H"

// For the entire time this object is instantiated, it holds a lock 
// on the specified lock.
class LockHolder
{
public:
  LockHolder(ILockable & lockToHold)
    throw (exception);

  virtual ~LockHolder();

  // Convenience methods to let owner of this LockHolder temporarily give up the
  // lock. In some cases, this can result in much cleaner code.
  void release()
    throw (exception);

  void reacquire()
    throw (exception);

private:
  ILockable & _lock;
  bool _presentlyHeld;
};

#endif
