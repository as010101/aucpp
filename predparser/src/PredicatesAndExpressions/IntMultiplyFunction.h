#ifndef INT_MULTIPLY_FUNCTION_H
#define INT_MULTIPLY_FUNCTION_H

#include "Function.H"

class IntMultiplyFunction : public Function
{
public:
  IntMultiplyFunction (Expression *left_side, Expression *right_side);
  IntMultiplyFunction(Expression *expr);
  virtual ~IntMultiplyFunction();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif

