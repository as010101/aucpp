#include "TsGTEPredicate.H"

TsGTEPredicate::TsGTEPredicate(Expression *left_side, Expression *right_side) 
{
  _ls = left_side;
  _rs = right_side;
}

TsGTEPredicate::~TsGTEPredicate() {}

bool TsGTEPredicate::evaluate(char *tuple) 
{
  char* left = _ls->evaluate(tuple);
  char* right = _rs->evaluate(tuple);
  
  bool b =  (*(Timestamp*)left) > (*(Timestamp*)right) ||
            (*(Timestamp*)left) == (*(Timestamp*)right);
  delete [] left;
  delete [] right;
  return b;
}

bool TsGTEPredicate::evaluate(char *tuple1, char *tuple2) 
{
  char* left = _ls->evaluate(tuple1, tuple2);
  char* right = _rs->evaluate(tuple1, tuple2);
  
  bool b =  (*(Timestamp*)left) > (*(Timestamp*)right) ||
            (*(Timestamp*)left) == (*(Timestamp*)right);
  delete [] left;
  delete [] right;
  return b;
}

TsGTEPredicate::TsGTEPredicate(Expression *expr) 
{
     _ls = expr;
}

void TsGTEPredicate::setExpression(Expression *expr) 
{
  if (!_ls)
    _ls = expr;
  else
    _rs = expr;
}
