#include "FloatAbsoluteFunction.H"

FloatAbsoluteFunction::FloatAbsoluteFunction(Expression *expr)
{
  _expr = expr;
}

FloatAbsoluteFunction::FloatAbsoluteFunction() {}

FloatAbsoluteFunction::~FloatAbsoluteFunction() {}

char* FloatAbsoluteFunction::evaluate(char *tuple)
{
  char *temp = _expr->evaluate(tuple);
  float tmp1 = *(float*) temp;
  float tmp2 = abs(tmp1);
  char *result = new char[sizeof(float)];
  *(float*) result = tmp2;
  delete[] temp;
  return result;
}

char* FloatAbsoluteFunction::evaluate(char *tuple1, char *tuple2)
{
  char *temp = _expr->evaluate(tuple1, tuple2);
  float tmp1 = *(float*) temp;
  float tmp2 = abs(tmp1);
  char *result = new char[sizeof(float)];
  *(float*) result = tmp2;
  delete[] temp;
  return result;
}

int FloatAbsoluteFunction::getReturnedSize()
{
  return sizeof(float);
}

void FloatAbsoluteFunction::setExpression(Expression *expr)
{
  _expr = expr;
}

