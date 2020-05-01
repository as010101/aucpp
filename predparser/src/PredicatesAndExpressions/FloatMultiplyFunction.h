#ifndef FLOAT_MULTIPLY_FUNCTION_H
#define FLOAT_MULTIPLY_FUNCTION_H

#include "Function.H"
#include <stdio.h>

class FloatMultiplyFunction : public Function
{
public:
  FloatMultiplyFunction (Expression *left_side, Expression *right_side);
  FloatMultiplyFunction(Expression *expr);
  virtual ~FloatMultiplyFunction();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif

