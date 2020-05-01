#ifndef STRING_GREATER_THAN_PREDICATE_H
#define STRING_GREATER_THAN_PREDICATE_H

#include "Predicate.H"

class StringGTPredicate : public Predicate
{
public:
  StringGTPredicate (Expression *left_side, Expression *right_side);
  StringGTPredicate (Expression *exp);  
  virtual ~StringGTPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
