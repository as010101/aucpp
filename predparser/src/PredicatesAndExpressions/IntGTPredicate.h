#ifndef INT_GREATER_THAN_PREDICATE_H
#define INT_GREATER_THAN_PREDICATE_H

#include "Predicate.H"

class IntGTPredicate : public Predicate
{
public:
  IntGTPredicate (Expression *left_side, Expression *right_side);
  IntGTPredicate(Expression *exp);  
  virtual ~IntGTPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
