#ifndef INT_SUBTRACT_FUNCTION_H
#define INT_SUBTRACT_FUNCTION_H

#include "Function.H"

class IntSubtractFunction : public Function
{
public:
  IntSubtractFunction (Expression *left_side, Expression *right_side);
  IntSubtractFunction(Expression *expr);
  virtual ~IntSubtractFunction();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();


private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif

