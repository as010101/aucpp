#ifndef FLOAT_ADD_FUNCTION_H
#define FLOAT_ADD_FUNCTION_H

#include "Function.H"
#include <stdio.h>

class FloatAddFunction : public Function
{
public:
  FloatAddFunction (Expression *left_side, Expression *right_side);
  FloatAddFunction(Expression *expr);
  virtual ~FloatAddFunction();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif

