#ifndef FLOAT_TO_INT_H
#define FLOAT_TO_INT_H
 
#include "Function.H"
 
class Float2Int : public Function
{
public:
  Float2Int (Expression *expr); 
  Float2Int();
  virtual ~Float2Int();
  void setExpression(Expression *expr);
  virtual char* evaluate(char *tuple);
  virtual char* evaluate(char *tuple1, char *tuple2);
  int getReturnedSize();
 
private:
  Expression    *_expr;

};
 
#endif
