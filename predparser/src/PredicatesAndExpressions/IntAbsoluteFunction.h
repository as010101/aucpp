#ifndef INT_ABSOLUTE_FUNCTION_H
#define INT_ABSOLUTE_FUNCTION_H

#include "Function.H"

class IntAbsoluteFunction : public Function
{
public:
  IntAbsoluteFunction(Expression *expr);
  IntAbsoluteFunction();
  virtual ~IntAbsoluteFunction();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();
  
private:
  Expression    *_expr;
  
};

#endif
