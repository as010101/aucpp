#ifndef INT_GREATER_THAN_OR_EQUAL_PREDICATE_H
#define INT_GREATER_THAN_OR_EQUAL_PREDICATE_H

#include "Predicate.H"

class IntGTEPredicate : public Predicate
{
public:
  IntGTEPredicate (Expression *left_side, Expression *right_side);
  IntGTEPredicate(Expression *exp);  
  virtual ~IntGTEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif
