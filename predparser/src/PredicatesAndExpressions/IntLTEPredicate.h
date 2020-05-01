#ifndef INT_LESS_THAN_OR_EQUAL_PREDICATE_H
#define INT_LESS_THAN_OR_EQUAL_PREDICATE_H

#include "Predicate.H"

class IntLTEPredicate : public Predicate
{
public:
  IntLTEPredicate (Expression *left_side, Expression *right_side);
  IntLTEPredicate(Expression *exp);  
  virtual ~IntLTEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif
