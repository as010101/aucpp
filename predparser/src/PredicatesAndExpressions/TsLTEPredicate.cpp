#include "TsLTEPredicate.H"

TsLTEPredicate::TsLTEPredicate(Expression *left_side, Expression *right_side) 
{
  _ls = left_side;
  _rs = right_side;
}

TsLTEPredicate::~TsLTEPredicate() {}

bool TsLTEPredicate::evaluate(char *tuple) 
{
  char* left = _ls->evaluate(tuple);
  char* right = _rs->evaluate(tuple);
  
  bool b =  (*(Timestamp*)left) < (*(Timestamp*)right) || 
            (*(Timestamp*)left) == (*(Timestamp*)right);
  delete [] left;
  delete [] right;
  return b;
}

bool TsLTEPredicate::evaluate(char *tuple1, char *tuple2) 
{
  char* left = _ls->evaluate(tuple1, tuple2);
  char* right = _rs->evaluate(tuple1, tuple2);
  
  bool b =   (*(Timestamp*)left) < (*(Timestamp*)right) ||
             (*(Timestamp*)left) == (*(Timestamp*)right);
  delete [] left;
  delete [] right;
  return b;
}

TsLTEPredicate::TsLTEPredicate(Expression *expr) 
{
     _ls = expr;
}

void TsLTEPredicate::setExpression(Expression *expr) 
{
  if (!_ls)
    _ls = expr;
  else
    _rs = expr;
}
