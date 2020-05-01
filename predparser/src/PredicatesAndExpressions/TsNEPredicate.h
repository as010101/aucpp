#ifndef TIMESTAMP_NOT_EQUAL_PREDICATE_H
#define TIMESTAMP_NOT_EQUAL_PREDICATE_H

#include "Predicate.H"

class TsNEPredicate : public Predicate
{
public:
  TsNEPredicate (Expression *left_side, Expression *right_side);
  TsNEPredicate(Expression *exp);  
  virtual ~TsNEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif
