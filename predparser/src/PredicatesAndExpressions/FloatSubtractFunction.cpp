#include "FloatSubtractFunction.H"

FloatSubtractFunction::FloatSubtractFunction(Expression *left_side, Expression *right_side) 
{
  _ls = left_side;
  _rs = right_side;
}

FloatSubtractFunction::~FloatSubtractFunction() {}

char* FloatSubtractFunction::evaluate(char *tuple) 
{
  char* left = _ls->evaluate(tuple);
  char* right = _rs->evaluate(tuple);
  float tmp1 = *(float*)(left);
  float tmp2 = *(float*)(right);
  char* result = new char[sizeof(float)];
  *(float*)(result) = tmp1 - tmp2;
  delete[] left;
  delete[] right;
  return  result;
}
 
char* FloatSubtractFunction::evaluate(char *tuple1, char *tuple2) 
{
  char* left = _ls->evaluate(tuple1, tuple2);
  char* right = _rs->evaluate(tuple1, tuple2);
  float tmp1 = *(float*)(left);
  float tmp2 = *(float*)(right);
  char* result = new char[sizeof(float)];
  *(float*)(result) = tmp1 - tmp2;
  delete[] left;
  delete[] right;
  return  result;
}
 
FloatSubtractFunction::FloatSubtractFunction(Expression *expr) 
{
     _ls = expr;
}

void FloatSubtractFunction::setExpression(Expression *expr) {
  if (!_ls)
    _ls = expr;
  else
    _rs = expr;
}

int FloatSubtractFunction::getReturnedSize()
{
  return sizeof(float);
}
