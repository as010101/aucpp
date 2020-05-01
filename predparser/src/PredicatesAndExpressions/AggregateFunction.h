#ifndef AGGREGATEFUNCTION_H
#define AGGREGATEFUNCTION_H

#include "Function.H"

class AggregateFunction : public Function
{
public:
  virtual void init() = 0;
  virtual void incr(char *tuple) = 0;
  virtual char* final() = 0;
  virtual int getReturnedSize() = 0;
  virtual AggregateFunction* makeNew() = 0;
};

#endif 
