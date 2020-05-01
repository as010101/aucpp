#ifndef FLOAT_LESS_THAN_OR_EQUAL_PREDICATE_H
#define FLOAT_LESS_THAN_OR_EQUAL_PREDICATE_H

#include "Predicate.H"

class FloatLTEPredicate : public Predicate
{
public:
  FloatLTEPredicate (Expression *left_side, Expression *right_side);
  FloatLTEPredicate(Expression *exp);  
  virtual ~FloatLTEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif
