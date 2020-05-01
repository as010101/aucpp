#include "TsEqualPredicate.H"

TsEqualPredicate::TsEqualPredicate(Expression *left_side, Expression *right_side) 
{
  _ls = left_side;
  _rs = right_side;
}

TsEqualPredicate::~TsEqualPredicate() {}

bool TsEqualPredicate::evaluate(char *tuple) 
{
  char* left = _ls->evaluate(tuple);
  char* right = _rs->evaluate(tuple);
  
  bool b = (*(Timestamp*)left) == (*(Timestamp*)right);
  delete [] left;
  delete [] right;
  return b;
}

bool TsEqualPredicate::evaluate(char *tuple1, char *tuple2) 
{
  char* left = _ls->evaluate(tuple1, tuple2);
  char* right = _rs->evaluate(tuple1, tuple2);
  
  bool b = (*(Timestamp*)left) == (*(Timestamp*)right);
  delete [] left;
  delete [] right;
  return b;
}

TsEqualPredicate::TsEqualPredicate(Expression *expr) 
{
     _ls = expr;
}

void TsEqualPredicate::setExpression(Expression *expr) 
{
  if (!_ls)
    _ls = expr;
  else
    _rs = expr;
}
