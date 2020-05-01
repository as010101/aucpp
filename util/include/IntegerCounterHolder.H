#ifndef IntegerCountHolder_H
#define IntegerCountHolder_H

#include "IntegerCounter.H"

class IntegerCounterHolder
{
public:
  IntegerCounterHolder(IntegerCounter & aCounter, int maxAllowableCount)
    throw (std::exception);

  virtual ~IntegerCounterHolder();

private:
  IntegerCounter & _counter;
};

#endif
