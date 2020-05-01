#ifndef FLOAT_GREATER_THAN_OR_EQUAL_PREDICATE_H
#define FLOAT_GREATER_THAN_OR_EQUAL_PREDICATE_H

#include "Predicate.H"

class FloatGTEPredicate : public Predicate
{
public:
  FloatGTEPredicate (Expression *left_side, Expression *right_side);
  FloatGTEPredicate(Expression *exp);  
  virtual ~FloatGTEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
