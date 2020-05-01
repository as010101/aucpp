#include "IntEqualPredicate.H"


IntEqualPredicate::IntEqualPredicate(Expression *left_side, Expression *right_side) 
{
  _ls = left_side;
  _rs = right_side;
}

IntEqualPredicate::~IntEqualPredicate() {}

bool IntEqualPredicate::evaluate(char *tuple) 
{
  char* left = _ls->evaluate(tuple);
  char* right = _rs->evaluate(tuple);
  
  bool b = (*(int*)left) == (*(int*)right);
  delete [] left;
  delete [] right;
  return b;
}

bool IntEqualPredicate::evaluate(char *tuple1, char *tuple2) 
{
  char* left = _ls->evaluate(tuple1, tuple2);
  char* right = _rs->evaluate(tuple1, tuple2);
  
  bool b = (*(int*)left) == (*(int*)right);
  delete [] left;
  delete [] right;
  return b;
}

IntEqualPredicate::IntEqualPredicate(Expression *expr) 
{
     _ls = expr;
}

void IntEqualPredicate::setExpression(Expression *expr) 
{
  if (!_ls)
    _ls = expr;
  else
    _rs = expr;
}
