#ifndef INT_EQUAL_PREDICATE_H
#define INT_EQUAL_PREDICATE_H

#include "Predicate.H"

class IntEqualPredicate : public Predicate
{
public:
  IntEqualPredicate (Expression *left_side, Expression *right_side);
  IntEqualPredicate(Expression *exp);  
  virtual ~IntEqualPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
