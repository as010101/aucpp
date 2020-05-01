#include "StringLTPredicate.H"

StringLTPredicate::StringLTPredicate(Expression *left_side, Expression *right_side) 
{
  _ls = left_side;
  _rs = right_side;
}

StringLTPredicate::~StringLTPredicate() {}

bool StringLTPredicate::evaluate(char *tuple) 
{
  char* left = _ls->evaluate(tuple);
  char* right = _rs->evaluate(tuple);
  bool b = (strcmp(left, right) < 0);
  delete[] left;
  delete[] right;
  return b;
}

bool StringLTPredicate::evaluate(char *tuple1, char *tuple2) 
{
  char* left = _ls->evaluate(tuple1, tuple2);
  char* right = _rs->evaluate(tuple1, tuple2);
  bool b = (strcmp(left, right) < 0);
  delete[] left;
  delete[] right;
  return b;
}

StringLTPredicate::StringLTPredicate(Expression *expr) 
{
     _ls = expr;
}

void StringLTPredicate::setExpression(Expression *expr) 
{
  if (!_ls)
    _ls = expr;
  else
    _rs = expr;
}
