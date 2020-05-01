#ifndef FUNCTION_H
#define FUNCTION_H

#include "Expression.H"
#include <stdlib.h>

class Function : public Expression
{
public:
  virtual char* evaluate(char *tuple) = 0;
  virtual char* evaluate(char *tuple1, char *tuple2) = 0;
  virtual int getReturnedSize() = 0;
};

#endif 
