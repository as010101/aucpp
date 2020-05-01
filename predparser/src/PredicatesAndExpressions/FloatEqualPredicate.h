#ifndef FLOAT_EQUAL_PREDICATE_H
#define FLOAT_EQUAL_PREDICATE_H

#include "Predicate.H"

class FloatEqualPredicate : public Predicate
{
public:
  FloatEqualPredicate (Expression *left_side, Expression *right_side);
  FloatEqualPredicate(Expression *exp);  
  virtual ~FloatEqualPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
