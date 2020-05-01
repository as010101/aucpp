#ifndef STRING_NOT_EQUAL_PREDICATE_H
#define STRING_NOT_EQUAL_PREDICATE_H

#include "Predicate.H"

class StringNEPredicate : public Predicate
{
public:
  StringNEPredicate (Expression *left_side, Expression *right_side);
  StringNEPredicate (Expression *exp);  
  virtual ~StringNEPredicate();
  void setExpression(Expression *exp);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif
