#ifndef INT_TO_FLOAT_H
#define INT_TO_FLOAT_H
 
#include "Function.H"
 
class Int2Float : public Function
{
public:
  Int2Float (Expression *expr); 
  Int2Float();
  virtual ~Int2Float();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();
 
private:
  Expression    *_expr;

};
 
#endif
