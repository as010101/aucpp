#ifndef TIMESTAMP_LESS_THAN_OR_EQUAL_PREDICATE_H
#define TIMESTAMP_LESS_THAN_OR_EQUAL_PREDICATE_H

#include "Predicate.H"

class TsLTEPredicate : public Predicate
{
public:
  TsLTEPredicate (Expression *left_side, Expression *right_side);
  TsLTEPredicate(Expression *exp);  
  virtual ~TsLTEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
