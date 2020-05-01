#ifndef FLOAT_SUBTRACT_FUNCTION_H
#define FLOAT_SUBTRACT_FUNCTION_H

#include "Function.H"
#include <stdio.h>

class FloatSubtractFunction : public Function
{
public:
  FloatSubtractFunction (Expression *left_side, Expression *right_side);
  FloatSubtractFunction(Expression *expr);
  virtual ~FloatSubtractFunction();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif

