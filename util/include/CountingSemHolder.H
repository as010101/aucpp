#ifndef CountingSemHolder_H
#define CountingSemHolder_H

#include "CountingSem.H"

// For the entire time this object is instantiated, it holds a lock 
// on the specified CountingSem.
class CountingSemHolder
{
public:
  CountingSemHolder(CountingSem & semToHold)
    throw (std::exception);

  virtual ~CountingSemHolder();
private:
  CountingSem & _sem;
};

#endif
