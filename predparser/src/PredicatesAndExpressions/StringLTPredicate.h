#ifndef STRING_LESS_THAN_PREDICATE_H
#define STRING_LESS_THAN_PREDICATE_H

#include "Predicate.H"

class StringLTPredicate : public Predicate
{
public:
  StringLTPredicate (Expression *left_side, Expression *right_side);
  StringLTPredicate (Expression *exp);  
  virtual ~StringLTPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
