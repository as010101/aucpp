#ifndef FLOAT_DIVIDE_FUNCTION_H
#define FLOAT_DIVIDE_FUNCTION_H

#include "Function.H"
#include <stdio.h>

class FloatDivideFunction : public Function
{
public:
  FloatDivideFunction (Expression *left_side, Expression *right_side);
  FloatDivideFunction(Expression *expr);
  virtual ~FloatDivideFunction();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif

