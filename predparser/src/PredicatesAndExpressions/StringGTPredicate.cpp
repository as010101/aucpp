#include "StringGTPredicate.H"

StringGTPredicate::StringGTPredicate(Expression *left_side, Expression *right_side) 
{
  _ls = left_side;
  _rs = right_side;
}

StringGTPredicate::~StringGTPredicate() {}

bool StringGTPredicate::evaluate(char *tuple) 
{
  char* left = _ls->evaluate(tuple);
  char* right = _rs->evaluate(tuple);
  bool b = (strcmp(left, right) > 0);
  delete[] left;
  delete[] right;
  return b;
}

bool StringGTPredicate::evaluate(char *tuple1, char *tuple2) 
{
  char* left = _ls->evaluate(tuple1, tuple2);
  char* right = _rs->evaluate(tuple1, tuple2);
  bool b = (strcmp(left, right) > 0);
  delete[] left;
  delete[] right;
  return b;
}

StringGTPredicate::StringGTPredicate(Expression *expr) 
{
     _ls = expr;
}

void StringGTPredicate::setExpression(Expression *expr) 
{
  if (!_ls)
    _ls = expr;
  else
    _rs = expr;
}
