#ifndef INT_LESS_THAN_PREDICATE_H
#define INT_LESS_THAN_PREDICATE_H

#include "Predicate.H"

class IntLTPredicate : public Predicate
{
public:
  IntLTPredicate (Expression *left_side, Expression *right_side);
  IntLTPredicate(Expression *expr);  
  virtual ~IntLTPredicate();
  void setExpression(Expression *expr);
  virtual bool evaluate(char *tuple);
  virtual bool evaluate(char *tuple1, char *tuple2);

private:
  Expression    *_ls;
  Expression    *_rs;
};

#endif
