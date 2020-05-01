#ifndef ILOCKABLE_H
#define ILOCKABLE_H

#include <exception>

using namespace std;

/**
 This is an interface that defines something as lockable. It's meant to let a
 variety of classes be served by the LockHolder class.
 */
class ILockable
{
public:
  virtual void lock()
    throw (std::exception) = 0;
  
  virtual void unlock()
    throw (std::exception) = 0;
};

#endif
