#include "FloatLTPredicate.H"

FloatLTPredicate::FloatLTPredicate(Expression *left_side, Expression *right_side) 
{
  _ls = left_side;
  _rs = right_side;
}

FloatLTPredicate::~FloatLTPredicate() {}

bool FloatLTPredicate::evaluate(char *tuple) 
{
  char* left = _ls->evaluate(tuple);
  char* right = _rs->evaluate(tuple);
  
  bool b = (*(float*)left) < (*(float*)right);
  delete [] left;
  delete [] right;
  return b;
}

bool FloatLTPredicate::evaluate(char *tuple1, char *tuple2) 
{
  char* left = _ls->evaluate(tuple1, tuple2);
  char* right = _rs->evaluate(tuple1, tuple2);
  
  bool b = (*(float*)left) < (*(float*)right);
  delete [] left;
  delete [] right;
  return b;
}

FloatLTPredicate::FloatLTPredicate(Expression *expr) 
{
     _ls = expr;
}

void FloatLTPredicate::setExpression(Expression *expr) 
{
  if (!_ls)
    _ls = expr;
  else
    _rs = expr;
}
