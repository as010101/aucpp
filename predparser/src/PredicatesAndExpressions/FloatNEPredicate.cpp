#include "FloatNEPredicate.H"

FloatNEPredicate::FloatNEPredicate(Expression *left_side, Expression *right_side) 
{
  _ls = left_side;
  _rs = right_side;
}

FloatNEPredicate::~FloatNEPredicate() {}

bool FloatNEPredicate::evaluate(char *tuple) 
{
  char* left = _ls->evaluate(tuple);
  char* right = _rs->evaluate(tuple);
  
  bool b = (*(float*)left) != (*(float*)right);
  delete [] left;
  delete [] right;
  return b;
}

bool FloatNEPredicate::evaluate(char *tuple1, char *tuple2) 
{
  char* left = _ls->evaluate(tuple1, tuple2);
  char* right = _rs->evaluate(tuple1, tuple2);
  
  bool b = (*(float*)left) != (*(float*)right);
  delete [] left;
  delete [] right;
  return b;
}

FloatNEPredicate::FloatNEPredicate(Expression *expr) {
     _ls = expr;
}

void FloatNEPredicate::setExpression(Expression *expr) {
  if (!_ls)
    _ls = expr;
  else
    _rs = expr;
}
