#ifndef FLOAT_CONSTANT_H
#define FLOAT_CONSTANT_H

#include "Expression.H"
#include <stdio.h>

class FloatConstant : public Expression
{
public:
  FloatConstant (float mem);
  virtual ~FloatConstant();
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();

private:
  float     _mem;
};

#endif
