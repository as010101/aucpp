#ifndef TIMESTAMP_GREATER_THAN_OR_EQUAL_PREDICATE_H
#define TIMESTAMP_GREATER_THAN_OR_EQUAL_PREDICATE_H

#include "Predicate.H"

class TsGTEPredicate : public Predicate
{
public:
  TsGTEPredicate (Expression *left_side, Expression *right_side);
  TsGTEPredicate(Expression *exp);  
  virtual ~TsGTEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression     *_ls;
  Expression     *_rs;
};

#endif
