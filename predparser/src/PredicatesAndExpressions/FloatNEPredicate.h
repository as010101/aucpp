#ifndef FLOAT_NOT_EQUAL_PREDICATE_H
#define FLOAT_NOT_EQUAL_PREDICATE_H

#include "Predicate.H"

class FloatNEPredicate : public Predicate
{
public:
  FloatNEPredicate (Expression *left_side, Expression *right_side);
  FloatNEPredicate(Expression *exp);  
  virtual ~FloatNEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
