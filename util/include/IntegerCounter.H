#ifndef IntegerCounter_H
#define IntegerCounter_H

#include "PtMutex.H"

/**
 An int, protected by a mutex. You're supposed to hold the mutex whenever
 you access the int.
 */
class IntegerCounter
{
public:
  IntegerCounter()
    throw (std::exception);

  virtual ~IntegerCounter();

  // If the current value of the int is < maxValue, this will increment the int
  // by one and return true.
  // If incrementing the int would make its value exceed 'maxValue', then this
  // will return false and will not modify the int.
  bool incrementWithTest(int maxValue)
    throw (std::exception);

  // Decrements the int. If decrementing the value would make the int negative, then
  // this is a no-op, and throws an exception.
  void decrement()
    throw (std::exception);

  int getValue()
    throw (std::exception);

private:
  IntegerCounter(const IntegerCounter & rhs);
  IntegerCounter & operator= (const IntegerCounter & rhs);

  PtMutex * _pMtx;
  int _intValue;
};

#endif
