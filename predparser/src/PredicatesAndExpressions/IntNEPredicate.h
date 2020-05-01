#ifndef INT_NOT_EQUAL_PREDICATE_H
#define INT_NOT_EQUAL_PREDICATE_H

#include "Predicate.H"

class IntNEPredicate : public Predicate
{
public:
  IntNEPredicate (Expression *left_side, Expression *right_side);
  IntNEPredicate(Expression *exp);  
  virtual ~IntNEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif
