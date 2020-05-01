#ifndef INT_ADD_FUNCTION_H
#define INT_ADD_FUNCTION_H

#include "Function.H"

class IntAddFunction : public Function
{
public:
  IntAddFunction (Expression *left_side, Expression *right_side);
  IntAddFunction(Expression *expr);
  virtual ~IntAddFunction();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif

