#ifndef FLOAT_GREATER_THAN_PREDICATE_H
#define FLOAT_GREATER_THAN_PREDICATE_H

#include "Predicate.H"

class FloatGTPredicate : public Predicate
{
public:
  FloatGTPredicate (Expression *left_side, Expression *right_side);
  FloatGTPredicate(Expression *exp);  
  virtual ~FloatGTPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);
private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
