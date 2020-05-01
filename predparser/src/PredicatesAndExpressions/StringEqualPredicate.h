#ifndef STRING_EQUAL_PREDICATE_H
#define STRING_EQUAL_PREDICATE_H

#include "Predicate.H"

class StringEqualPredicate : public Predicate
{
public:
  StringEqualPredicate (Expression *left_side, Expression *right_side);
  StringEqualPredicate (Expression *exp);  
  virtual ~StringEqualPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression   *_ls;
  Expression   *_rs;
};

#endif
