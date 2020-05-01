#ifndef STRING_LESS_THAN_OR_EQUAL_PREDICATE_H
#define STRING_LESS_THAN_OR_EQUAL_PREDICATE_H

#include "Predicate.H"

class StringLTEPredicate : public Predicate
{
public:
  StringLTEPredicate (Expression *left_side, Expression *right_side);
  StringLTEPredicate (Expression *exp);  
  virtual ~StringLTEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
