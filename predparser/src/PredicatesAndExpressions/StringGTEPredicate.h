#ifndef STRING_GREATER_THAN_OR_EQUAL_PREDICATE_H
#define STRING_GREATER_THAN_OR_EQUAL_PREDICATE_H

#include "Predicate.H"

class StringGTEPredicate : public Predicate
{
public:
  StringGTEPredicate (Expression *left_side, Expression *right_side);
  StringGTEPredicate (Expression *exp);  
  virtual ~StringGTEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
