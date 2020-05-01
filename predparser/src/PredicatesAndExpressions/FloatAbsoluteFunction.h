#ifndef FLOAT_ABSOLUTE_FUNCTION_H
#define FLOAT_ABSOLUTE_FUNCTION_H

#include "Function.H"

class FloatAbsoluteFunction : public Function
{
public:
  FloatAbsoluteFunction(Expression *expr);
  FloatAbsoluteFunction();
  virtual ~FloatAbsoluteFunction();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();
  
private:
  Expression    *_expr;
  
};

#endif
