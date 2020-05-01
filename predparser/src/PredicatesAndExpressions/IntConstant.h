#ifndef INT_CONSTANT_H
#define INT_CONSTANT_H

#include "Expression.H"

class IntConstant : public Expression
{
public:
  IntConstant (int mem);
  virtual ~IntConstant();
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();

private:
  int   _mem;
};

#endif
