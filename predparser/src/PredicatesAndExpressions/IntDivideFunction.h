#ifndef INT_DIVIDE_FUNCTION_H
#define INT_DIVIDE_FUNCTION_H

#include "Function.H"

class IntDivideFunction : public Function
{
public:
  IntDivideFunction (Expression *left_side, Expression *right_side);
  IntDivideFunction(Expression *expr);
  virtual ~IntDivideFunction();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif

