#include "IntAbsoluteFunction.H"

IntAbsoluteFunction::IntAbsoluteFunction(Expression *expr)
{
  _expr = expr;
}

IntAbsoluteFunction::IntAbsoluteFunction() {}

IntAbsoluteFunction::~IntAbsoluteFunction() {}

char* IntAbsoluteFunction::evaluate(char *tuple)
{
  char *temp = _expr->evaluate(tuple);
  int tmp1 = *(int*) temp;
  int tmp2 = abs(tmp1);
  char *result = new char[sizeof(int)];
  *(int*) result = tmp2;
  delete[] temp;
  return result;
}

char* IntAbsoluteFunction::evaluate(char *tuple1, char *tuple2)
{
  char *temp = _expr->evaluate(tuple1, tuple2);
  int tmp1 = *(int*) temp;
  int tmp2 = abs(tmp1);
  char *result = new char[sizeof(int)];
  *(int*) result = tmp2;
  delete[] temp;
  return result;
}

int IntAbsoluteFunction::getReturnedSize()
{
  return sizeof(int);
}

void IntAbsoluteFunction::setExpression(Expression *expr)
{
  _expr = expr;
}

