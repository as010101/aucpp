#include "IntAddFunction.H"

IntAddFunction::IntAddFunction(Expression *left_side, Expression *right_side) 
{
  _ls = left_side;
  _rs = right_side;
}

IntAddFunction::~IntAddFunction() {}

char* IntAddFunction::evaluate(char *tuple) 
{
  char* left = _ls->evaluate(tuple);
  char* right = _rs->evaluate(tuple);
  int tmp1 = *(int*)(left);
  int tmp2 = *(int*)(right);
  char* result = new char[sizeof(int)];
  *(int*)(result) = tmp1 + tmp2;
  delete[] left;
  delete[] right;
  return  result;
}

char* IntAddFunction::evaluate(char *tuple1, char *tuple2) 
{
  char* left = _ls->evaluate(tuple1, tuple2);
  char* right = _rs->evaluate(tuple1, tuple2);
  int tmp1 = *(int*)(left);
  int tmp2 = *(int*)(right);
  char* result = new char[sizeof(int)];
  *(int*)(result) = tmp1 + tmp2;
  delete[] left;
  delete[] right;
  return  result;
}

IntAddFunction::IntAddFunction(Expression *expr) 
{
     _ls = expr;
}

void IntAddFunction::setExpression(Expression *expr) 
{
  if (!_ls)
    _ls = expr;
  else
    _rs = expr;
}

int IntAddFunction::getReturnedSize()
{
  return sizeof(int);
}
